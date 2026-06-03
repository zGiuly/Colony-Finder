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

bool SimdjsonIndexer::TrimAndSkip(const char*& data, size_t& length)
{
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
    return false;
}

bool SimdjsonIndexer::ParseDocument(const char* data, size_t length, dom::object& obj)
{
    dom::element doc;
    auto parseErr = m_parser.parse(reinterpret_cast<const uint8_t*>(data), length, true).get(doc);
    if (parseErr) return false;
    if (doc.get_object().get(obj)) return false;
    return true;
}

bool SimdjsonIndexer::ProcessLine(const char* data, size_t length)
{
    if (m_cancel.load()) return false;
    if (TrimAndSkip(data, length)) return true;

    dom::object obj;
    if (!ParseDocument(data, length, obj)) return true;

    ParsedSystem parsed;
    for (const auto& field : obj)
    {
        HandleField(field.key, field.value, parsed);
    }

    if (!parsed.hasId || !parsed.hasName || !parsed.hasCoords) return true;

    WriteRecord(parsed);
    return true;
}

void SimdjsonIndexer::HandleField(std::string_view key, dom::element value, ParsedSystem& out)
{
    using Handler = void (SimdjsonIndexer::*)(dom::element, ParsedSystem&);
    struct Route { std::string_view key; Handler fn; };
    static const Route table[] = {
        { "id64",               &SimdjsonIndexer::HandleId },
        { "name",               &SimdjsonIndexer::HandleName },
        { "coords",             &SimdjsonIndexer::HandleCoords },
        { "population",         &SimdjsonIndexer::HandlePopulation },
        { "bodyCount",          &SimdjsonIndexer::HandleBodyCount },
        { "controllingFaction", &SimdjsonIndexer::HandleControllingFaction },
        { "bodies",             &SimdjsonIndexer::HandleBodies },
    };

    for (const auto& r : table)
    {
        if (r.key != key) continue;
        (this->*r.fn)(value, out);
        return;
    }
}

void SimdjsonIndexer::HandleId(dom::element value, ParsedSystem& out)
{
    uint64_t v;
    if (value.get_uint64().get(v)) return;
    out.rec.id64 = v;
    out.hasId = true;
}

void SimdjsonIndexer::HandleName(dom::element value, ParsedSystem& out)
{
    std::string_view v;
    if (value.get_string().get(v)) return;
    out.name = v;
    out.hasName = true;
}

void SimdjsonIndexer::HandleCoords(dom::element value, ParsedSystem& out)
{
    dom::object coords;
    if (value.get_object().get(coords)) return;

    for (const auto& cf : coords)
    {
        double cv;
        if (cf.value.get_double().get(cv)) continue;
        if (cf.key.size() != 1) continue;

        switch (cf.key[0])
        {
            case 'x': out.rec.x = static_cast<float>(cv); break;
            case 'y': out.rec.y = static_cast<float>(cv); break;
            case 'z': out.rec.z = static_cast<float>(cv); break;
            default: break;
        }
    }
    out.hasCoords = true;
}

void SimdjsonIndexer::HandlePopulation(dom::element value, ParsedSystem& out)
{
    uint64_t v;
    if (value.get_uint64().get(v)) return;
    out.rec.population = v;
}

void SimdjsonIndexer::HandleBodyCount(dom::element value, ParsedSystem& out)
{
    uint64_t v;
    if (value.get_uint64().get(v)) return;
    out.rec.bodyCount = static_cast<uint16_t>(v);
}

void SimdjsonIndexer::HandleControllingFaction(dom::element value, ParsedSystem& out)
{
    dom::object fac;
    if (value.get_object().get(fac)) return;

    for (const auto& ff : fac)
    {
        if (ff.key != "isPlayer") continue;

        bool v;
        if (ff.value.get_bool().get(v)) continue;
        if (!v) continue;

        out.rec.flags |= SystemIndex::System_PlayerColonized;
    }
}

void SimdjsonIndexer::HandleBodies(dom::element value, ParsedSystem& out)
{
    dom::array bodies;
    if (value.get_array().get(bodies)) return;

    for (const auto& bodyElem : bodies)
    {
        dom::object body;
        if (bodyElem.get_object().get(body)) continue;
        HandleBody(body, out);
    }
}

void SimdjsonIndexer::HandleBody(dom::object body, ParsedSystem& out)
{
    std::string_view type;
    std::string_view subType;
    bool isLandable = false;
    bool hasType = false;
    bool hasSubType = false;

    enum class BodyField { Unknown, Type, SubType, IsLandable };
    auto classify = [](std::string_view k) {
        if (k == "type")       return BodyField::Type;
        if (k == "subType")    return BodyField::SubType;
        if (k == "isLandable") return BodyField::IsLandable;
        return BodyField::Unknown;
    };

    for (const auto& bf : body)
    {
        switch (classify(bf.key))
        {
            case BodyField::Type:
            {
                std::string_view v;
                if (bf.value.get_string().get(v)) break;
                type = v; hasType = true;
                break;
            }
            case BodyField::SubType:
            {
                std::string_view v;
                if (bf.value.get_string().get(v)) break;
                subType = v; hasSubType = true;
                break;
            }
            case BodyField::IsLandable:
            {
                bool v;
                if (bf.value.get_bool().get(v)) break;
                isLandable = v;
                break;
            }
            case BodyField::Unknown:
            default:
                break;
        }
    }

    if (!hasType || !hasSubType) return;

    enum class BodyKind { Star, Planet, Other };
    auto kind = [&]() {
        if (type == "Star")   return BodyKind::Star;
        if (type == "Planet") return BodyKind::Planet;
        return BodyKind::Other;
    }();

    switch (kind)
    {
        case BodyKind::Star:
            ParseStarType(subType, out.rec.starTypesMask);
            return;
        case BodyKind::Planet:
            ParseBodyType(subType, out.rec.bodyTypeCounts);
            if (isLandable) out.rec.flags |= SystemIndex::System_HasLandable;
            return;
        case BodyKind::Other:
        default:
            return;
    }
}

void SimdjsonIndexer::WriteRecord(const ParsedSystem& parsed)
{
    SystemIndex::Record rec = parsed.rec;
    rec.nameOffset = static_cast<uint32_t>(m_stringTableOffset);

    m_recordsFile.write(reinterpret_cast<const char*>(&rec), sizeof(rec));
    m_stringsFile.write(parsed.name.data(), parsed.name.size());
    char zero = '\0';
    m_stringsFile.write(&zero, 1);
    m_stringTableOffset += parsed.name.size() + 1;
    m_totalSystems++;
}

void SimdjsonIndexer::ParseStarType(std::string_view subType, uint16_t& mask)
{
    struct Entry { std::string_view needle; uint16_t bit; };
    static constexpr Entry table[] = {
        { "Neutron Star", SystemIndex::Star_Neutron },
        { "Black Hole",   SystemIndex::Star_BlackHole },
        { "White Dwarf",  SystemIndex::Star_WhiteDwarf },
        { "O (",          SystemIndex::Star_O },
        { "B (",          SystemIndex::Star_B },
        { "A (",          SystemIndex::Star_A },
        { "F (",          SystemIndex::Star_F },
        { "G (",          SystemIndex::Star_G },
        { "K (",          SystemIndex::Star_K },
        { "M (",          SystemIndex::Star_M },
    };

    for (const auto& e : table)
    {
        if (subType.find(e.needle) == std::string_view::npos) continue;
        mask |= e.bit;
        return;
    }

    if (subType.find("L (") != std::string_view::npos ||
        subType.find("T (") != std::string_view::npos ||
        subType.find("Y (") != std::string_view::npos ||
        subType.find("Brown dwarf") != std::string_view::npos)
    {
        mask |= SystemIndex::Star_LTY;
        return;
    }

    mask |= SystemIndex::Star_Other;
}

void SimdjsonIndexer::ParseBodyType(std::string_view subType, uint8_t (&counts)[8])
{
    struct Entry { std::string_view needle; int idx; };
    static const Entry table[] = {
        { "Earth-like world",          SystemIndex::BTI_ELW },
        { "Water world",               SystemIndex::BTI_WW },
        { "Ammonia world",             SystemIndex::BTI_AMW },
        { "High metal content world",  SystemIndex::BTI_HMC },
        { "Metal-rich body",           SystemIndex::BTI_MetalRich },
        { "Rocky body",                SystemIndex::BTI_Rocky },
    };

    int idx = -1;
    for (const auto& e : table)
    {
        if (subType.find(e.needle) == std::string_view::npos) continue;
        idx = e.idx;
        break;
    }

    if (idx < 0 && (subType.find("Icy body") != std::string_view::npos ||
                    subType.find("Rocky Ice world") != std::string_view::npos))
    {
        idx = SystemIndex::BTI_Icy;
    }
    if (idx < 0 && (subType.find("gas giant") != std::string_view::npos ||
                    subType.find("Gas giant") != std::string_view::npos))
    {
        idx = SystemIndex::BTI_GasGiant;
    }

    if (idx < 0) return;
    if (counts[idx] >= 255) return;
    counts[idx]++;
}
