#include "search/IndexBuilderSax.h"
#include <algorithm>

enum JsonDepth
{
    Depth_Root = 1,
    Depth_System = 2,
    Depth_CoordsOrBodies = 3,
    Depth_Body = 4,
    Depth_Signals = 5
};

IndexBuilderSax::IndexBuilderSax(std::ofstream& recFile, std::ofstream& strFile, uint64_t& systemCount, uint64_t& strOffset, std::atomic<bool>& cancelFlag)
    : recordsFile(recFile),
      stringsFile(strFile),
      totalSystems(systemCount),
      stringTableOffset(strOffset),
      cancel(cancelFlag),
      depth(0),
      inCoords(false),
      inBodies(false),
      inBody(false),
      currentBodyIsLandable(false)
{
}

bool IndexBuilderSax::null()
{
    return !cancel.load();
}

bool IndexBuilderSax::boolean(bool val)
{
    if (depth == Depth_Body && inBody && currentKey == "isLandable")
    {
        currentBodyIsLandable = val;
    }
    return !cancel.load();
}

bool IndexBuilderSax::number_integer(number_integer_t val)
{
    if (depth == Depth_System)
    {
        if (currentKey == "id64")
        {
            currentSystem.id64 = val;
        }
        else if (currentKey == "population")
        {
            currentSystem.population = val;
        }
        else if (currentKey == "bodyCount")
        {
            currentSystem.bodyCount = static_cast<uint16_t>(val);
        }
    }
    else if (depth == Depth_Signals)
    {
        if (currentKey == "Biological" && val > 0)
        {
            currentSystem.bodyTypesMask |= SystemIndex::Body_BioSignals;
        }
        else if (currentKey == "Geological" && val > 0)
        {
            currentSystem.bodyTypesMask |= SystemIndex::Body_GeoSignals;
        }
    }
    return !cancel.load();
}

bool IndexBuilderSax::number_unsigned(number_unsigned_t val)
{
    if (depth == Depth_System)
    {
        if (currentKey == "id64")
        {
            currentSystem.id64 = val;
        }
        else if (currentKey == "population")
        {
            currentSystem.population = val;
        }
        else if (currentKey == "bodyCount")
        {
            currentSystem.bodyCount = static_cast<uint16_t>(val);
        }
    }
    else if (depth == Depth_Signals)
    {
        if (currentKey == "Biological" && val > 0)
        {
            currentSystem.bodyTypesMask |= SystemIndex::Body_BioSignals;
        }
        else if (currentKey == "Geological" && val > 0)
        {
            currentSystem.bodyTypesMask |= SystemIndex::Body_GeoSignals;
        }
    }
    return !cancel.load();
}

bool IndexBuilderSax::number_float(number_float_t val, const string_t&)
{
    if (depth == Depth_CoordsOrBodies && inCoords)
    {
        if (currentKey == "x")
        {
            currentSystem.x = static_cast<float>(val);
        }
        else if (currentKey == "y")
        {
            currentSystem.y = static_cast<float>(val);
        }
        else if (currentKey == "z")
        {
            currentSystem.z = static_cast<float>(val);
        }
    }
    return !cancel.load();
}

bool IndexBuilderSax::string(string_t& val)
{
    if (depth == Depth_System)
    {
        if (currentKey == "name")
        {
            currentSystem.name = val;
        }
    }
    else if (depth == Depth_Body && inBody)
    {
        if (currentKey == "type")
        {
            currentBodyType = val;
        }
        else if (currentKey == "subType")
        {
            currentBodySubType = val;
        }
    }
    return !cancel.load();
}

bool IndexBuilderSax::binary(binary_t&)
{
    return !cancel.load();
}

bool IndexBuilderSax::start_object(std::size_t)
{
    depth++;
    if (depth == Depth_System)
    {
        currentSystem = CurrentSystem();
    }
    else if (depth == Depth_Body && inBodies)
    {
        inBody = true;
        currentBodyType.clear();
        currentBodySubType.clear();
        currentBodyIsLandable = false;
    }
    return !cancel.load();
}

bool IndexBuilderSax::key(string_t& val)
{
    currentKey = val;
    if (depth == Depth_System && currentKey == "coords")
    {
        inCoords = true;
    }
    return !cancel.load();
}

bool IndexBuilderSax::end_object()
{
    if (depth == Depth_System)
    {
        SystemIndex::Record rec;
        rec.id64 = currentSystem.id64;
        rec.x = currentSystem.x;
        rec.y = currentSystem.y;
        rec.z = currentSystem.z;
        rec.population = currentSystem.population;
        rec.bodyCount = currentSystem.bodyCount;
        rec.starTypesMask = currentSystem.starTypesMask;
        rec.bodyTypesMask = currentSystem.bodyTypesMask;
        rec.nameOffset = static_cast<uint32_t>(stringTableOffset);
        rec.flags = 0;
        rec.reserved = 0;

        recordsFile.write(reinterpret_cast<const char*>(&rec), sizeof(rec));
        stringsFile.write(currentSystem.name.c_str(), currentSystem.name.size() + 1);
        stringTableOffset += currentSystem.name.size() + 1;
        totalSystems++;
    }
    else if (depth == Depth_Body && inBody)
    {
        inBody = false;
        if (currentBodyType == "Star")
        {
            ParseStarType(currentBodySubType, currentSystem.starTypesMask);
        }
        else if (currentBodyType == "Planet")
        {
            ParseBodyType(currentBodySubType, currentSystem.bodyTypesMask);
            if (currentBodyIsLandable)
            {
                currentSystem.bodyTypesMask |= SystemIndex::Body_Landable;
            }
        }
    }
    depth--;
    return !cancel.load();
}

bool IndexBuilderSax::start_array(std::size_t)
{
    depth++;
    if (depth == Depth_CoordsOrBodies && currentKey == "bodies")
    {
        inBodies = true;
    }
    return !cancel.load();
}

bool IndexBuilderSax::end_array()
{
    if (depth == Depth_CoordsOrBodies && inBodies)
    {
        inBodies = false;
    }
    depth--;
    return !cancel.load();
}

bool IndexBuilderSax::parse_error(std::size_t, const std::string&, const nlohmann::detail::exception&)
{
    return false;
}

void IndexBuilderSax::ParseStarType(const std::string& subType, uint16_t& mask)
{
    if (subType.find("Neutron Star") != std::string::npos)
    {
        mask |= SystemIndex::Star_Neutron;
    }
    else if (subType.find("Black Hole") != std::string::npos)
    {
        mask |= SystemIndex::Star_BlackHole;
    }
    else if (subType.find("White Dwarf") != std::string::npos)
    {
        mask |= SystemIndex::Star_WhiteDwarf;
    }
    else if (subType.find("O (") != std::string::npos)
    {
        mask |= SystemIndex::Star_O;
    }
    else if (subType.find("B (") != std::string::npos)
    {
        mask |= SystemIndex::Star_B;
    }
    else if (subType.find("A (") != std::string::npos)
    {
        mask |= SystemIndex::Star_A;
    }
    else if (subType.find("F (") != std::string::npos)
    {
        mask |= SystemIndex::Star_F;
    }
    else if (subType.find("G (") != std::string::npos)
    {
        mask |= SystemIndex::Star_G;
    }
    else if (subType.find("K (") != std::string::npos)
    {
        mask |= SystemIndex::Star_K;
    }
    else if (subType.find("M (") != std::string::npos)
    {
        mask |= SystemIndex::Star_M;
    }
    else if (subType.find("L (") != std::string::npos || subType.find("T (") != std::string::npos || subType.find("Y (") != std::string::npos || subType.find("Brown dwarf") != std::string::npos)
    {
        mask |= SystemIndex::Star_LTY;
    }
    else
    {
        mask |= SystemIndex::Star_Other;
    }
}

void IndexBuilderSax::ParseBodyType(const std::string& subType, uint32_t& mask)
{
    if (subType.find("Earth-like world") != std::string::npos)
    {
        mask |= SystemIndex::Body_ELW;
    }
    else if (subType.find("Water world") != std::string::npos)
    {
        mask |= SystemIndex::Body_WW;
    }
    else if (subType.find("Ammonia world") != std::string::npos)
    {
        mask |= SystemIndex::Body_AMW;
    }
    else if (subType.find("High metal content world") != std::string::npos)
    {
        mask |= SystemIndex::Body_HMC;
    }
    else if (subType.find("Metal-rich body") != std::string::npos)
    {
        mask |= SystemIndex::Body_MetalRich;
    }
    else if (subType.find("Rocky body") != std::string::npos)
    {
        mask |= SystemIndex::Body_Rocky;
    }
    else if (subType.find("Icy body") != std::string::npos || subType.find("Rocky Ice world") != std::string::npos)
    {
        mask |= SystemIndex::Body_Icy;
    }
    else if (subType.find("gas giant") != std::string::npos || subType.find("Gas giant") != std::string::npos)
    {
        mask |= SystemIndex::Body_GasGiant;
    }
}
