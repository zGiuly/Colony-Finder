#include "search/SimdjsonIndexer.h"
#include "search/SystemIndex.h"

using namespace simdjson;

SimdjsonIndexer::SimdjsonIndexer(std::ofstream& recFile,
                                 std::ofstream& strFile,
                                 uint64_t& totalSystems,
                                 uint64_t& stringTableOffset,
                                 std::atomic<bool>& cancelFlag)
    : m_recordsFile(recFile),
      m_stringsFile(strFile),
      m_totalSystems(totalSystems),
      m_stringTableOffset(stringTableOffset),
      m_cancel(cancelFlag)
{
}

bool SimdjsonIndexer::ProcessLine(const char* data, size_t length)
{
    if (m_cancel.load()) return false;

    while (length > 0 && (*data == ' ' || *data == '\t' || *data == '\r'))
    {
        ++data;
        --length;
    }
    while (length > 0 && (data[length - 1] == ' ' || data[length - 1] == '\t' || data[length - 1] == '\r' || data[length - 1] == ','))
    {
        --length;
    }
    if (length == 0) return true;
    if (length == 1 && (*data == '[' || *data == ']')) return true;

    dom::element doc;
    auto parseErr = m_parser.parse(reinterpret_cast<const uint8_t*>(data), length, true).get(doc);
    if (parseErr) return true;

    dom::object obj;
    if (doc.get_object().get(obj)) return true;

    SystemIndex::Record rec{};
    std::string_view name;
    bool hasId = false;
    bool hasName = false;
    bool hasCoords = false;

    for (const auto& field : obj)
    {
        std::string_view key = field.key;
        auto value = field.value;

        if (key == "id64")
        {
            uint64_t v;
            if (!value.get_uint64().get(v)) { rec.id64 = v; hasId = true; }
        }
        else if (key == "name")
        {
            std::string_view v;
            if (!value.get_string().get(v)) { name = v; hasName = true; }
        }
        else if (key == "coords")
        {
            dom::object coords;
            if (!value.get_object().get(coords))
            {
                for (const auto& cf : coords)
                {
                    double cv;
                    if (cf.value.get_double().get(cv)) continue;
                    if (cf.key == "x") rec.x = static_cast<float>(cv);
                    else if (cf.key == "y") rec.y = static_cast<float>(cv);
                    else if (cf.key == "z") rec.z = static_cast<float>(cv);
                }
                hasCoords = true;
            }
        }
        else if (key == "population")
        {
            uint64_t v;
            if (!value.get_uint64().get(v)) rec.population = v;
        }
        else if (key == "bodyCount")
        {
            uint64_t v;
            if (!value.get_uint64().get(v)) rec.bodyCount = static_cast<uint16_t>(v);
        }
        else if (key == "controllingFaction")
        {
            dom::object fac;
            if (!value.get_object().get(fac))
            {
                for (const auto& ff : fac)
                {
                    if (ff.key == "isPlayer")
                    {
                        bool v;
                        if (!ff.value.get_bool().get(v) && v)
                        {
                            rec.flags |= SystemIndex::System_PlayerColonized;
                        }
                    }
                }
            }
        }
        else if (key == "bodies")
        {
            dom::array bodies;
            if (value.get_array().get(bodies)) continue;
            for (const auto& bodyElem : bodies)
            {
                dom::object body;
                if (bodyElem.get_object().get(body)) continue;

                std::string_view type;
                std::string_view subType;
                bool isLandable = false;
                bool hasType = false;
                bool hasSubType = false;

                for (const auto& bf : body)
                {
                    if (bf.key == "type")
                    {
                        std::string_view v;
                        if (!bf.value.get_string().get(v)) { type = v; hasType = true; }
                    }
                    else if (bf.key == "subType")
                    {
                        std::string_view v;
                        if (!bf.value.get_string().get(v)) { subType = v; hasSubType = true; }
                    }
                    else if (bf.key == "isLandable")
                    {
                        bool v;
                        if (!bf.value.get_bool().get(v)) isLandable = v;
                    }
                }

                if (hasType && hasSubType)
                {
                    if (type == "Star")
                    {
                        ParseStarType(subType, rec.starTypesMask);
                    }
                    else if (type == "Planet")
                    {
                        ParseBodyType(subType, rec.bodyTypeCounts);
                        if (isLandable) rec.flags |= SystemIndex::System_HasLandable;
                    }
                }
            }
        }
    }

    if (!hasId || !hasName || !hasCoords) return true;

    rec.nameOffset = static_cast<uint32_t>(m_stringTableOffset);

    m_recordsFile.write(reinterpret_cast<const char*>(&rec), sizeof(rec));
    m_stringsFile.write(name.data(), name.size());
    char zero = '\0';
    m_stringsFile.write(&zero, 1);
    m_stringTableOffset += name.size() + 1;
    m_totalSystems++;

    return true;
}

void SimdjsonIndexer::ParseStarType(std::string_view subType, uint16_t& mask)
{
    if (subType.find("Neutron Star") != std::string_view::npos) mask |= SystemIndex::Star_Neutron;
    else if (subType.find("Black Hole") != std::string_view::npos) mask |= SystemIndex::Star_BlackHole;
    else if (subType.find("White Dwarf") != std::string_view::npos) mask |= SystemIndex::Star_WhiteDwarf;
    else if (subType.find("O (") != std::string_view::npos) mask |= SystemIndex::Star_O;
    else if (subType.find("B (") != std::string_view::npos) mask |= SystemIndex::Star_B;
    else if (subType.find("A (") != std::string_view::npos) mask |= SystemIndex::Star_A;
    else if (subType.find("F (") != std::string_view::npos) mask |= SystemIndex::Star_F;
    else if (subType.find("G (") != std::string_view::npos) mask |= SystemIndex::Star_G;
    else if (subType.find("K (") != std::string_view::npos) mask |= SystemIndex::Star_K;
    else if (subType.find("M (") != std::string_view::npos) mask |= SystemIndex::Star_M;
    else if (subType.find("L (") != std::string_view::npos ||
             subType.find("T (") != std::string_view::npos ||
             subType.find("Y (") != std::string_view::npos ||
             subType.find("Brown dwarf") != std::string_view::npos) mask |= SystemIndex::Star_LTY;
    else mask |= SystemIndex::Star_Other;
}

void SimdjsonIndexer::ParseBodyType(std::string_view subType, uint8_t (&counts)[8])
{
    int idx = -1;
    if (subType.find("Earth-like world") != std::string_view::npos) idx = SystemIndex::BTI_ELW;
    else if (subType.find("Water world") != std::string_view::npos) idx = SystemIndex::BTI_WW;
    else if (subType.find("Ammonia world") != std::string_view::npos) idx = SystemIndex::BTI_AMW;
    else if (subType.find("High metal content world") != std::string_view::npos) idx = SystemIndex::BTI_HMC;
    else if (subType.find("Metal-rich body") != std::string_view::npos) idx = SystemIndex::BTI_MetalRich;
    else if (subType.find("Rocky body") != std::string_view::npos) idx = SystemIndex::BTI_Rocky;
    else if (subType.find("Icy body") != std::string_view::npos ||
             subType.find("Rocky Ice world") != std::string_view::npos) idx = SystemIndex::BTI_Icy;
    else if (subType.find("gas giant") != std::string_view::npos ||
             subType.find("Gas giant") != std::string_view::npos) idx = SystemIndex::BTI_GasGiant;

    if (idx >= 0 && counts[idx] < 255) counts[idx]++;
}
