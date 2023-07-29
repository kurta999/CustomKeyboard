#include "pch.hpp"

uint8_t GetPaletteIndexFromRGB(uint8_t from_r, uint8_t from_g, uint8_t from_b);
uint16_t GetWeaponModel(int weaponid);

class GtaEntityBase
{
public:
    GtaEntityBase(const std::string& name_, Vec3D pos_, Vec3D rot_, uint8_t interior_, int world_) :
        name(name_), pos(pos_), rot(rot_), interior(interior_), world(world_) {}
    virtual ~GtaEntityBase() { }

    virtual std::string ConvertToSampFormat(MapConverterFlags flags, Vec3D offset) = 0;

    std::string name;
    Vec3D pos;
    Vec3D rot;
    uint8_t interior = 0;
    int world = -1;

protected:
    [[nodiscard]] std::string GetComment(MapConverterFlags flags)
    {
        std::string ret = (name.empty() || !(flags & MapConverterFlags::CONVERT_ELEMENT_NAME)) ? "\n" : std::format(" // {}\n", name);
        return ret;
    }
};

class GtaObject final : public GtaEntityBase
{
public:
    GtaObject(const std::string& name_, uint16_t model_id_, Vec3D pos_, Vec3D rot_, uint8_t interior_, int world_, uint8_t alpha_) :
        model_id(model_id_), GtaEntityBase(name_, pos_, rot_, interior_, world_), alpha(alpha_) { }

    GtaObject(const std::string& name_, uint16_t model_id_, const std::string& pos_, const std::string& rot_) :
        model_id(model_id_), GtaEntityBase(name_, Vec3D(), Vec3D(), 0, 0) 
    {
        std::vector<std::string> vec_pos(3);
        boost::split(vec_pos, pos_, [](char input) { return input == ' ' || input == ','; }, boost::algorithm::token_compress_on);
        pos = Vec3D(boost::lexical_cast<float>(vec_pos[0]), boost::lexical_cast<float>(vec_pos[1]), boost::lexical_cast<float>(vec_pos[2])); 
        
        std::vector<std::string> vec_rot(3);
        boost::split(vec_rot, rot_, [](char input) { return input == ' ' || input == ','; }, boost::algorithm::token_compress_on);
        rot = Vec3D(boost::lexical_cast<float>(vec_rot[0]), boost::lexical_cast<float>(vec_rot[1]), boost::lexical_cast<float>(vec_rot[2]));
    }

    std::string ConvertToSampFormat(MapConverterFlags flags, Vec3D offset) override
    {
        std::string ret;
        if((flags & MapConverterFlags::HIDE_WHEN_ALPHA_NOT_255) && alpha != 0xFF)
            ret += "\tSetDynamicObjectMaterial(CreateDynamicObject(";
        else
            ret += "\tCreateDynamicObject(";

        ret += std::format("{}, {:.3f}, {:.3f}, {:.3f}, {:.3f}, {:.3f}, {:.3f}", model_id, pos.x + offset.x, pos.y + offset.y, pos.z + offset.z, rot.x, rot.y, rot.z);

        if(world != 0)
            ret += std::format(", {}", world);

        if(interior != 0)
        {
            if(world != 0)
                ret += ", -1";
            ret += std::format(", {}", interior);
        }

        if((flags & MapConverterFlags::HIDE_WHEN_ALPHA_NOT_255) && alpha != 0xFF)
            ret += "), 0, 0, \"a\", \"a\", 0x000000FF);";
        else
            ret += ");";
        ret += GetComment(flags);
        return ret;
    }  

    uint16_t model_id = 0;
    uint8_t alpha = 0;
};

class GtaRemoveWorldObject final : public GtaEntityBase
{
public:
    GtaRemoveWorldObject(const std::string& name_, uint16_t model_id_, uint16_t lod_model_, float radius_, Vec3D pos_, uint8_t interior_) :
        model_id(model_id_), lod_model(lod_model_), radius(radius_), GtaEntityBase(name_, pos_, Vec3D(0.0f, 0.0f, 0.0f), interior_, -1) { }

    std::string ConvertToSampFormat(MapConverterFlags flags, Vec3D offset) override
    {
        std::string ret;
        ret += std::format("\tRemoveBuildingForPlayer(playerid, {}, {:.3f}, {:.3f}, {:.3f}, {:.3f});", model_id, pos.x + offset.x, pos.y + offset.y, pos.z + offset.z, radius);
        ret += GetComment(flags);
        if(lod_model != 0)
        {
            ret += std::format("\tRemoveBuildingForPlayer(playerid, {}, {:.3f}, {:.3f}, {:.3f}, {:.3f});", lod_model, pos.x + offset.x, pos.y + offset.y, pos.z + offset.z, radius);
            ret += GetComment(flags);
        }
        return ret;
    }

    uint16_t model_id = 0;
    uint16_t lod_model = 0;
    float radius = 0.0f;
};

class GtaVehicle final : public GtaEntityBase
{
public:
    GtaVehicle(const std::string& name_, uint16_t model_id_, const std::string& colors_, uint8_t paintjob_, const std::string& upgrades_, const std::string& plate_,
        Vec3D pos_, float angle, uint8_t interior_, int world_) :
        model_id(model_id_), paintjob(paintjob_), plate(plate_), GtaEntityBase(name_, pos_, Vec3D(0.0f, 0.0f, angle), interior_, world_)
    {
        if(!upgrades_.empty())
        {
            std::vector<std::string> vec_upgrades(14);
            boost::split(vec_upgrades, upgrades_, [](char input) { return input == ' ' || input == ','; }, boost::algorithm::token_compress_on);
            for(auto& i : vec_upgrades)
                upgrades.push_back(utils::stoi<uint16_t>(i));
        }

        if(colors_ != "-1")
        {
            std::vector<std::string> vec_colors(12);
            boost::split(vec_colors, colors_, [](char input) { return input == ' ' || input == ','; }, boost::algorithm::token_compress_on);
            try
            {
                for(auto& i : vec_colors)
                    colors.push_back(utils::stoi<uint8_t>(i));
            }
            catch(const std::exception& e)
            {
                LOG(LogLevel::Error, "Invalid color format, exception: {}! Input string: {}", e.what(), colors_);
                for(uint8_t i = 0; i != 12 - colors.size(); i++)
                    colors.push_back(utils::random_mt(0, 255));  /* Use random colors as recovery */
            }
        }
        else
        {
            for(uint8_t i = 0; i != 12; i++)
                colors.push_back(utils::random_mt(0, 255));  /* Use random colors */
        }
    }

    GtaVehicle(const std::string& name_, uint16_t model_id_, const std::string& pos_, float angle) :
        model_id(model_id_), GtaEntityBase(name_, Vec3D(), Vec3D(0.0f, 0.0f, angle), 0, 0)
    {
        std::vector<std::string> vec_pos(3);
        boost::split(vec_pos, pos_, [](char input) { return input == ' ' || input == ','; }, boost::algorithm::token_compress_on);
        pos = Vec3D(boost::lexical_cast<float>(vec_pos[0]), boost::lexical_cast<float>(vec_pos[1]), boost::lexical_cast<float>(vec_pos[2]));
    }

    std::string ConvertToSampFormat(MapConverterFlags flags, Vec3D offset) override
    {
        std::string ret;
        int16_t color_1 = -1, color_2 = -1;

        if(!colors.empty())
        {
            if(colors.size() > 4)  /* New MTA - use HEX colors */
            {
                color_1 = GetPaletteIndexFromRGB(colors[0], colors[1], colors[2]);
                color_2 = GetPaletteIndexFromRGB(colors[3], colors[4], colors[5]);
            }
            else
            {
                if(colors.size() >= 2)
                {
                    color_1 = colors[0];
                    color_2 = colors[1];
                }
                else
                {
                    LOG(LogLevel::Error, "Invalid color vector size! {}", colors.size());
                }
            }
        }

        bool no_specialization = flags & MapConverterFlags::ONLY_CREATE_VEHICLE || (!interior && !world && paintjob >= 4 && upgrades.empty());
        ret += std::format("\t{}CreateVehicle({}, {:.3f}, {:.3f}, {:.3f}, {:.3f}, {}, {}, -1);", no_specialization ? "" : "tempvehid = ", 
            model_id, pos.x + offset.x, pos.y + offset.y, pos.z + offset.z, rot.z, color_1, color_2);
        ret += GetComment(flags);

        if(!no_specialization)
        {
            if(flags & MapConverterFlags::SAVE_NUMBER_PLATE && !plate.empty())
                ret += std::format("\tSetVehicleNumberPlate(tempvehid, \"{}\");\n", plate);

            if(interior != 0)
                ret += std::format("\tLinkVehicleToInterior(tempvehid, {});\n", interior);
            if(world != 0)
                ret += std::format("\tSetVehicleVirtualWorld(tempvehid, {});\n", world);
            if(paintjob < 3)
                ret += std::format("\tChangeVehiclePaintjob(tempvehid, {});\n", paintjob);

            if(!upgrades.empty())
            {
                if(!(flags & MapConverterFlags::VEHICLE_COMPONENTS_INLINE))
                {
                    for(auto i : upgrades)
                        ret += std::format("\tAddVehicleComponent(tempvehid, {});\n", i);
                }
                else
                {
                    ret += std::format("\tAddVehicleComponentInline(tempvehid, ");
                    for(auto i : upgrades)
                        ret += std::to_string(i) + ", ";
                    if(ret.size() > 2 && ret.back() == ' ')
                    {
                        ret.pop_back();
                        ret.pop_back();
                    }
                    ret += ");\n";
                }
            }
        }

        return ret;
    }

    uint16_t model_id = 0;
    std::vector<uint8_t> colors;
    uint8_t paintjob = 4;
    std::vector<uint16_t> upgrades;
    std::string plate;
};

class GtaMarker final : public GtaEntityBase
{
public:
    GtaMarker(const std::string& name_, const std::string&type_, Vec3D pos_, float size_, const std::string& color_, uint8_t interior_, int world) :
        GtaEntityBase(name_, pos_, Vec3D(), interior_, world), size(size_)
    { 
        if(!type_.empty())
        {
            // "arrow", "checkpoint", "corona", "cylinder", "ring"
            switch(type_[1])
            {	
                case 'r': type = MarkerType::ARROW; break;
                case 'h': type = MarkerType::CHECKPOINT; break;
                case 'o': type = MarkerType::CORONA; break;
                case 'y': type = MarkerType::CYLINDER; break;
                case 'i': type = MarkerType::RING; break;
                default:
                {
                    LOG(LogLevel::Warning, "GtaMarker invalid type: {}", type_);
                    break;
                }
            }
         }
        color = std::strtoul(color_.c_str(), 0, 16);
    }

    GtaMarker(const std::string& name_, const std::string& type_, const std::string& pos_, float size_, const std::string& color_) :
        GtaEntityBase(name_, Vec3D(), Vec3D(), 0, 0), size(size_)
    {
            std::vector<std::string> vec_pos(3);
            boost::split(vec_pos, pos_, [](char input) { return input == ' ' || input == ','; }, boost::algorithm::token_compress_on);
            pos = Vec3D(boost::lexical_cast<float>(vec_pos[0]), boost::lexical_cast<float>(vec_pos[1]), boost::lexical_cast<float>(vec_pos[2]));

            std::vector<std::string> vec_rgb(3);
            boost::split(vec_rgb, pos_, [](char input) { return input == ' ' || input == ','; }, boost::algorithm::token_compress_on);

            // "arrow", "checkpoint", "corona", "cylinder", "ring"
            if(!type_.empty())
            {
                switch(type_[1])
                {
                    case 'r': type = MarkerType::ARROW; break;
                    case 'h': type = MarkerType::CHECKPOINT; break;
                    case 'o': type = MarkerType::CORONA; break;
                    case 'y': type = MarkerType::CYLINDER; break;
                    case 'i': type = MarkerType::RING; break;
                    default:
                    {
                        LOG(LogLevel::Warning, "GtaMarker invalid type: {}", type_);
                        break;
                    }
                }
            }
            else
            {
                type = MarkerType::CYLINDER;
            }

            color = utils::RGB_TO_INT(std::stoi(vec_rgb[0]), std::stoi(vec_rgb[1]), std::stoi(vec_rgb[2]));
    }

    std::string ConvertToSampFormat(MapConverterFlags flags, Vec3D offset) override
    {
        std::string ret;
        switch(type)
        {
            case MarkerType::ARROW:  // arrow	
            {
                ret += std::format("\tSetDynamicObjectMaterial(CreateDynamicObject(19180, {:.3f}, {:.3f}, {:.3f}, 0.0, 0.0, 0.0), 0, 0, \"a\", \"a\", 0x{:X});",
                    pos.x + offset.x, pos.y + offset.y, pos.z + offset.z, color);
                break;
            }
            case MarkerType::CHECKPOINT:  // checkpoint
            {
                ret += std::format("\tCreateDynamicCP({:.3f}, {:.3f}, {:.3f}, {:.3f}", pos.x + offset.x, pos.y + offset.y, pos.z + offset.z, size);
                break;
            }
            case MarkerType::CORONA:  // corona
            {
                LOG(LogLevel::Warning, "Corona isn't supported yet!");
                // I'll do it, if somebody give me an object which is good to imitate corona (aka object which is lighting without object, like the sun)
                break;
            }
            case MarkerType::CYLINDER:  // cylinder (default CP)
            {
                ret += std::format("\tCreateDynamicRaceCP(2, {:.3f}, {:.3f}, {:.3f}, 0.0, 0.0, 0.0, {:.3f}", pos.x + offset.x, pos.y + offset.y, pos.z + offset.z, size);
                break;
            }
            case MarkerType::RING:  // ring
            {
                ret += std::format("\tCreateDynamicRaceCP(4, {:.3f}, {:.3f}, {:.3f}, 0.0, 0.0, 0.0, {:.3f}", pos.x + offset.x, pos.y + offset.y, pos.z + offset.z, size);
                break;
            }
        }

        if(type != MarkerType::CORONA)
        {
            if(type != MarkerType::ARROW)
            {
                if(world != 0)
                    ret += std::format(", {}", world);

                if(interior != 0)
                {
                    if(world != 0)
                        ret += ", -1";
                    ret += std::format(", {}", interior);
                }
                
                ret += ");";
            }
            ret += GetComment(flags);
        }

        return ret;
    }

    enum class MarkerType : uint8_t
    {
        ARROW,
        CHECKPOINT,
        CORONA,
        CYLINDER,
        RING,
        INVALID
    };

    MarkerType type = MarkerType::INVALID;
    float size = 0.0f;
    uint16_t color = 0;
};

class GtaPickup final : public GtaEntityBase
{
public:
    GtaPickup(const std::string& name_, const std::string& type, Vec3D pos_, uint8_t interior_, int world) :
        GtaEntityBase(name_, pos_, Vec3D(0.0f, 0.0f, 0.0f), interior_, world)
    {
        if(utils::is_number(type))
        {
            uint16_t weaponid = GetWeaponModel(utils::stoi<int>(type));
            model_id = weaponid ? weaponid : utils::stoi<int>(type);
        }
        else
        {
            if(type == "health")
                model_id = 1240;
            else if(type == "armor")
                model_id = 1242;
            else
            {
                LOG(LogLevel::Warning, "GtaPickup invalid type: {}", type);
            }
        }
    }

    GtaPickup(const std::string& name_, const std::string& type, const std::string& pos_) :
        GtaEntityBase(name_, Vec3D(), Vec3D(), 0, 0)
    {
        std::vector<std::string> vec_pos(3);
        boost::split(vec_pos, pos_, [](char input) { return input == ' ' || input == ','; }, boost::algorithm::token_compress_on);
        pos = Vec3D(boost::lexical_cast<float>(vec_pos[0]), boost::lexical_cast<float>(vec_pos[1]), boost::lexical_cast<float>(vec_pos[2]));

        if(type == "nitro")  /* TODO: find the appropiate SA-MP model id */
            model_id = 1;
        else if(type == "repair")
            model_id = 2;
    }

    std::string ConvertToSampFormat(MapConverterFlags flags, Vec3D offset) override
    {
        std::string ret;
        ret += std::format("\tCreateDynamicPickup({}, 2, {:.3f}, {:.3f}, {:.3f}", model_id, pos.x + offset.x, pos.y + offset.y, pos.z + offset.z);
        if(world != 0)
            ret += std::format(", {}", world);

        if(interior != 0)
        {
            if(world != 0)
                ret += ", -1";
            ret += std::format(", {}", interior);
        }
        ret += ");";
        ret += GetComment(flags);
        return ret;
    }

    uint16_t model_id = 0;
};

class GtaPed final : public GtaEntityBase
{
public:
    GtaPed(const std::string& name_, uint16_t model_id_, Vec3D pos_, float angle, uint8_t interior_, int world) :
        model_id(model_id_), GtaEntityBase(name_, pos_, Vec3D(0.0f, 0.0f, angle), interior_, world)
    {
    }

    std::string ConvertToSampFormat(MapConverterFlags flags, Vec3D offset) override
    {
        std::string ret;
        ret += std::format("\tCreateActor({}, {:.3f}, {:.3f}, {:.3f}", model_id, pos.x + offset.x, pos.y + offset.y, pos.z + offset.z);
        if(world != 0)
            ret += std::format(", {}", world);

        if(interior != 0)
        {
            if(world != 0)
                ret += ", -1";
            ret += std::format(", {}", interior);
        }
        ret += ");";
        ret += GetComment(flags);
        return ret;
    }

    uint16_t model_id = 0;
};

std::string MapConverter::MtaToSamp(const std::string& input, MapConverterFlags flags, Vec3D offset)
{
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();

    std::string out;
    size_t all_items = 0;

    bool ret = true;
    boost::property_tree::ptree pt;
    std::unordered_map<ItemType, std::vector<std::unique_ptr<GtaEntityBase>>> items;

    try
    {
        std::stringstream ss;
        ss << input;
        boost::property_tree::read_xml(ss, pt);

        boost::optional<std::string> map_type_mta_dm = pt.get_optional<std::string>("map.<xmlattr>.edf:definitions");
        boost::optional<std::string> map_type_mta_race = pt.get_optional<std::string>("map.<xmlattr>.mod");
        if(map_type_mta_dm.has_value())  /* if this attribute is present, then it's an MTA DM map (editor_main is the default value) */
        {
            for(const boost::property_tree::ptree::value_type& v : pt.get_child("map"))
            {
                if(v.first == "object")
                {
                    auto opt_interior = v.second.get_child_optional("<xmlattr>.interior");  /* Older MTA DM versions doesn't have these attributes */
                    auto opt_dimension = v.second.get_child_optional("<xmlattr>.dimension");
                    auto opt_alpha = v.second.get_child_optional("<xmlattr>.alpha");
                    std::unique_ptr<GtaObject> item = std::make_unique<GtaObject>(v.second.get<std::string>("<xmlattr>.id"), v.second.get<uint16_t>("<xmlattr>.model"),
                        Vec3D(v.second.get<float>("<xmlattr>.posX"), v.second.get<float>("<xmlattr>.posY"), v.second.get<float>("<xmlattr>.posZ")),
                        Vec3D(v.second.get<float>("<xmlattr>.rotX"), v.second.get<float>("<xmlattr>.rotY"), v.second.get<float>("<xmlattr>.rotZ")),
                        opt_interior.has_value() ? opt_interior->get_value<int>() : 0,
                        opt_dimension.has_value() ? opt_dimension->get_value<int>() : 0, opt_alpha.has_value() ? opt_alpha->get_value<uint8_t>() : 0xFF);

                    items[ItemType::OBJECT].push_back(std::move(item));
                }
                else if(v.first == "removeWorldObject")
                {
                    std::unique_ptr<GtaRemoveWorldObject> item = std::make_unique<GtaRemoveWorldObject>(v.second.get<std::string>("<xmlattr>.id"), v.second.get<uint16_t>("<xmlattr>.model"),
                        v.second.get<uint16_t>("<xmlattr>.lodModel"), v.second.get<float>("<xmlattr>.radius"),
                        Vec3D(v.second.get<float>("<xmlattr>.posX"), v.second.get<float>("<xmlattr>.posY"), v.second.get<float>("<xmlattr>.posZ")),
                        v.second.get<int>("<xmlattr>.interior"));

                    items[ItemType::REMOVE_OBJECT].push_back(std::move(item));
                }
                else if(v.first == "vehicle")
                {
                    boost::optional<uint8_t> paintjob = v.second.get<uint8_t>("<xmlattr>.paintjob");
                    boost::optional<std::string> upgrades = v.second.get<std::string>("<xmlattr>.upgrades");
                    boost::optional<std::string> plate = v.second.get<std::string>("<xmlattr>.plate");
                    std::unique_ptr<GtaVehicle> item = std::make_unique<GtaVehicle>(v.second.get<std::string>("<xmlattr>.id"), v.second.get<uint16_t>("<xmlattr>.model"),
                        v.second.get<std::string>("<xmlattr>.color"), paintjob.has_value() ? *paintjob : 255, upgrades.has_value() ? *upgrades : "", 
                        plate.has_value() ? *plate : "",
                        Vec3D(v.second.get<float>("<xmlattr>.posX"), v.second.get<float>("<xmlattr>.posY"), v.second.get<float>("<xmlattr>.posZ")),
                        v.second.get<float>("<xmlattr>.rotZ"), v.second.get<int>("<xmlattr>.interior"), v.second.get<int>("<xmlattr>.dimension"));

                    items[ItemType::VEHICLE].push_back(std::move(item));
                }
                else if(v.first == "marker")
                {
                    std::unique_ptr<GtaMarker> item = std::make_unique<GtaMarker>(v.second.get<std::string>("<xmlattr>.id"), v.second.get<std::string>("<xmlattr>.type"),
                        Vec3D(v.second.get<float>("<xmlattr>.posX"), v.second.get<float>("<xmlattr>.posY"), v.second.get<float>("<xmlattr>.posZ")),
                        v.second.get<float>("<xmlattr>.size"), v.second.get<std::string>("<xmlattr>.color"), 
                        v.second.get<int>("<xmlattr>.interior"), v.second.get<int>("<xmlattr>.dimension"));

                    items[ItemType::MARKER].push_back(std::move(item));
                }                
                else if(v.first == "pickup")
                {
                    std::unique_ptr<GtaPickup> item = std::make_unique<GtaPickup>(v.second.get<std::string>("<xmlattr>.id"), v.second.get<std::string>("<xmlattr>.type"),
                        Vec3D(v.second.get<float>("<xmlattr>.posX"), v.second.get<float>("<xmlattr>.posY"), v.second.get<float>("<xmlattr>.posZ")),
                        v.second.get<int>("<xmlattr>.interior"), v.second.get<int>("<xmlattr>.dimension"));

                    items[ItemType::PICKUP].push_back(std::move(item));
                }
                else if(v.first == "ped")
                {
                    std::unique_ptr<GtaPed> item = std::make_unique<GtaPed>(v.second.get<std::string>("<xmlattr>.id"), v.second.get<uint16_t>("<xmlattr>.model"),
                        Vec3D(v.second.get<float>("<xmlattr>.posX"), v.second.get<float>("<xmlattr>.posY"), v.second.get<float>("<xmlattr>.posZ")),
                        v.second.get<float>("<xmlattr>.rotZ"), v.second.get<int>("<xmlattr>.interior"), v.second.get<int>("<xmlattr>.dimension"));

                    items[ItemType::PED].push_back(std::move(item));
                }
                else if(v.first == "spawnpoint")  /* Legacy MTA:DM */
                {
                    std::unique_ptr<GtaVehicle> item = std::make_unique<GtaVehicle>(v.second.get<std::string>("<xmlattr>.id"), v.second.get<uint16_t>("<xmlattr>.vehicle"),
                        "-1", 255, "", "",
                        Vec3D(v.second.get<float>("<xmlattr>.posX"), v.second.get<float>("<xmlattr>.posY"), v.second.get<float>("<xmlattr>.posZ")),
                        v.second.get<float>("<xmlattr>.rotZ"), 0, 0);

                    items[ItemType::VEHICLE].push_back(std::move(item));
                }
                else if(v.first == "checkpoint")  /* Legacy MTA:DM */
                {
                    std::unique_ptr<GtaMarker> item = std::make_unique<GtaMarker>(v.second.get<std::string>("<xmlattr>.id"), "checkpoint",
                        Vec3D(v.second.get<float>("<xmlattr>.posX"), v.second.get<float>("<xmlattr>.posY"), v.second.get<float>("<xmlattr>.posZ")),
                        v.second.get<float>("<xmlattr>.size"), v.second.get<std::string>("<xmlattr>.color"),
                        0, 0);

                    items[ItemType::MARKER].push_back(std::move(item));
                }
                else
                {
                    LOG(LogLevel::Warning, "Unsupported MTA:SA DM item type: {}", v.first);
                }
            }
        }
        else if(map_type_mta_race.has_value())
        {
            for(const boost::property_tree::ptree::value_type& v : pt.get_child("map"))
            {
                if(v.first == "spawnpoint")
                {
                    std::unique_ptr<GtaVehicle> item = std::make_unique<GtaVehicle>(v.second.get<std::string>("<xmlattr>.name"), v.second.get_child("vehicle").get_value<uint16_t>(),
                        v.second.get_child("position").get_value<std::string>(), v.second.get_child("rotation").get_value<float>());

                    items[ItemType::VEHICLE].push_back(std::move(item));
                }
                else if(v.first == "object")
                {
                    std::unique_ptr<GtaObject> item = std::make_unique<GtaObject>(v.second.get<std::string>("<xmlattr>.name"), v.second.get_child("model").get_value<uint16_t>(),
                        v.second.get_child("position").get_value<std::string>(), v.second.get_child("rotation").get_value<std::string>());

                    items[ItemType::OBJECT].push_back(std::move(item));
                }
                else if(v.first == "pickup")
                {
                    std::unique_ptr<GtaPickup> item = std::make_unique<GtaPickup>(v.second.get<std::string>("<xmlattr>.name"), v.second.get_child("type").get_value<std::string>(),
                        v.second.get_child("position").get_value<std::string>());

                    items[ItemType::PICKUP].push_back(std::move(item));
                }
                else if(v.first == "checkpoint")
                {
                    auto opt_type = v.second.get_child_optional("type");
                    auto opt_color = v.second.get_child_optional("color");
                    auto opt_size = v.second.get_child_optional("size");
                    std::unique_ptr<GtaMarker> item = std::make_unique<GtaMarker>(v.second.get<std::string>("<xmlattr>.name"), opt_type.has_value() ? opt_type->get_value<std::string>() : "",
                        v.second.get_child("position").get_value<std::string>(), opt_size.has_value() ? opt_size->get_value<float>() : 2.0f,
                        opt_color.has_value() ? opt_color->get_value<std::string>() : "255 255 255");

                    items[ItemType::MARKER].push_back(std::move(item));
                }
                else
                {
                    LOG(LogLevel::Warning, "Unsupported MTA:SA Race item type: {}", v.first);
                }
            }
            DBG("race");
        }

        for(auto& i : items)
        {
            for(auto& x : i.second)
            {
                out += x->ConvertToSampFormat(flags, offset);
            }
            out += "\n\n";
            all_items += i.second.size();
        }

        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
        int64_t dif = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        LOG(LogLevel::Notification, "{} items converted in {:.6f} ms (obj: {}, remove obj: {}, vehicle: {}, marker: {}, pickup: {}, ped: {})",
            all_items, (double)dif / 1000000.0, items[ItemType::OBJECT].size(), items[ItemType::REMOVE_OBJECT].size(), items[ItemType::VEHICLE].size(), items[ItemType::MARKER].size(),
            items[ItemType::PICKUP].size(), items[ItemType::PED].size());

        items.clear();
    }
    catch(boost::property_tree::xml_parser_error& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}, {}", e.filename(), e.what());
        ret = false;
    }
    catch(std::exception& e)
    {
        LOG(LogLevel::Error, "Exception thrown: {}", e.what());
        ret = false;
    }
    return out;
}

// https://github.com/multitheftauto/mtasa-blue/blob/55c5a42ee29a79554a3a09f3d19da5ced20ca742/Shared/sdk/SharedUtil.Game.hpp#L192
static const uint8_t paletteColorTable8[] = {
            0x00, 0x00, 0x00, 0xff, 0xf5, 0xf5, 0xf5, 0xff, 0x2a, 0x77, 0xa1, 0xff, 0x84, 0x04, 0x10, 0xff,
            0x26, 0x37, 0x39, 0xff, 0x86, 0x44, 0x6e, 0xff, 0xd7, 0x8e, 0x10, 0xff, 0x4c, 0x75, 0xb7, 0xff,
            0xbd, 0xbe, 0xc6, 0xff, 0x5e, 0x70, 0x72, 0xff, 0x46, 0x59, 0x7a, 0xff, 0x65, 0x6a, 0x79, 0xff,
            0x5d, 0x7e, 0x8d, 0xff, 0x58, 0x59, 0x5a, 0xff, 0xd6, 0xda, 0xd6, 0xff, 0x9c, 0xa1, 0xa3, 0xff,
            0x33, 0x5f, 0x3f, 0xff, 0x73, 0x0e, 0x1a, 0xff, 0x7b, 0x0a, 0x2a, 0xff, 0x9f, 0x9d, 0x94, 0xff,
            0x3b, 0x4e, 0x78, 0xff, 0x73, 0x2e, 0x3e, 0xff, 0x69, 0x1e, 0x3b, 0xff, 0x96, 0x91, 0x8c, 0xff,
            0x51, 0x54, 0x59, 0xff, 0x3f, 0x3e, 0x45, 0xff, 0xa5, 0xa9, 0xa7, 0xff, 0x63, 0x5c, 0x5a, 0xff,
            0x3d, 0x4a, 0x68, 0xff, 0x97, 0x95, 0x92, 0xff, 0x42, 0x1f, 0x21, 0xff, 0x5f, 0x27, 0x2b, 0xff,
            0x84, 0x94, 0xab, 0xff, 0x76, 0x7b, 0x7c, 0xff, 0x64, 0x64, 0x64, 0xff, 0x5a, 0x57, 0x52, 0xff,
            0x25, 0x25, 0x27, 0xff, 0x2d, 0x3a, 0x35, 0xff, 0x93, 0xa3, 0x96, 0xff, 0x6d, 0x7a, 0x88, 0xff,
            0x22, 0x19, 0x18, 0xff, 0x6f, 0x67, 0x5f, 0xff, 0x7c, 0x1c, 0x2a, 0xff, 0x5f, 0x0a, 0x15, 0xff,
            0x19, 0x38, 0x26, 0xff, 0x5d, 0x1b, 0x20, 0xff, 0x9d, 0x98, 0x72, 0xff, 0x7a, 0x75, 0x60, 0xff,
            0x98, 0x95, 0x86, 0xff, 0xad, 0xb0, 0xb0, 0xff, 0x84, 0x89, 0x88, 0xff, 0x30, 0x4f, 0x45, 0xff,
            0x4d, 0x62, 0x68, 0xff, 0x16, 0x22, 0x48, 0xff, 0x27, 0x2f, 0x4b, 0xff, 0x7d, 0x62, 0x56, 0xff,
            0x9e, 0xa4, 0xab, 0xff, 0x9c, 0x8d, 0x71, 0xff, 0x6d, 0x18, 0x22, 0xff, 0x4e, 0x68, 0x81, 0xff,
            0x9c, 0x9c, 0x98, 0xff, 0x91, 0x73, 0x47, 0xff, 0x66, 0x1c, 0x26, 0xff, 0x94, 0x9d, 0x9f, 0xff,
            0xa4, 0xa7, 0xa5, 0xff, 0x8e, 0x8c, 0x46, 0xff, 0x34, 0x1a, 0x1e, 0xff, 0x6a, 0x7a, 0x8c, 0xff,
            0xaa, 0xad, 0x8e, 0xff, 0xab, 0x98, 0x8f, 0xff, 0x85, 0x1f, 0x2e, 0xff, 0x6f, 0x82, 0x97, 0xff,
            0x58, 0x58, 0x53, 0xff, 0x9a, 0xa7, 0x90, 0xff, 0x60, 0x1a, 0x23, 0xff, 0x20, 0x20, 0x2c, 0xff,
            0xa4, 0xa0, 0x96, 0xff, 0xaa, 0x9d, 0x84, 0xff, 0x78, 0x22, 0x2b, 0xff, 0x0e, 0x31, 0x6d, 0xff,
            0x72, 0x2a, 0x3f, 0xff, 0x7b, 0x71, 0x5e, 0xff, 0x74, 0x1d, 0x28, 0xff, 0x1e, 0x2e, 0x32, 0xff,
            0x4d, 0x32, 0x2f, 0xff, 0x7c, 0x1b, 0x44, 0xff, 0x2e, 0x5b, 0x20, 0xff, 0x39, 0x5a, 0x83, 0xff,
            0x6d, 0x28, 0x37, 0xff, 0xa7, 0xa2, 0x8f, 0xff, 0xaf, 0xb1, 0xb1, 0xff, 0x36, 0x41, 0x55, 0xff,
            0x6d, 0x6c, 0x6e, 0xff, 0x0f, 0x6a, 0x89, 0xff, 0x20, 0x4b, 0x6b, 0xff, 0x2b, 0x3e, 0x57, 0xff,
            0x9b, 0x9f, 0x9d, 0xff, 0x6c, 0x84, 0x95, 0xff, 0x4d, 0x84, 0x95, 0xff, 0xae, 0x9b, 0x7f, 0xff,
            0x40, 0x6c, 0x8f, 0xff, 0x1f, 0x25, 0x3b, 0xff, 0xab, 0x92, 0x76, 0xff, 0x13, 0x45, 0x73, 0xff,
            0x96, 0x81, 0x6c, 0xff, 0x64, 0x68, 0x6a, 0xff, 0x10, 0x50, 0x82, 0xff, 0xa1, 0x99, 0x83, 0xff,
            0x38, 0x56, 0x94, 0xff, 0x52, 0x56, 0x61, 0xff, 0x7f, 0x69, 0x56, 0xff, 0x8c, 0x92, 0x9a, 0xff,
            0x59, 0x6e, 0x87, 0xff, 0x47, 0x35, 0x32, 0xff, 0x44, 0x62, 0x4f, 0xff, 0x73, 0x0a, 0x27, 0xff,
            0x22, 0x34, 0x57, 0xff, 0x64, 0x0d, 0x1b, 0xff, 0xa3, 0xad, 0xc6, 0xff, 0x69, 0x58, 0x53, 0xff,
            0x9b, 0x8b, 0x80, 0xff, 0x62, 0x0b, 0x1c, 0xff, 0x5b, 0x5d, 0x5e, 0xff, 0x62, 0x44, 0x28, 0xff,
            0x73, 0x18, 0x27, 0xff, 0x1b, 0x37, 0x6d, 0xff, 0xec, 0x6a, 0xae, 0xff,
};

// https://github.com/multitheftauto/mtasa-blue/blob/dd4d47a2efa93ba6030b9f8620de0faacb901d2b/Client/mods/deathmatch/logic/CClientPickupManager.cpp#L17
static const uint16_t g_usWeaponModels[] =
{
    0, 331, 333, 334, 335, 336, 337, 338, 339, 341,     // 9
    321, 322, 323, 0, 325, 326, 342, 343, 344, 0,       // 19
    0, 0, 346, 347, 348, 349, 350, 351, 352, 353,       // 29
    355, 356, 372, 357, 358, 359, 360, 361, 362, 363,   // 39
    364, 365, 366, 367, 368, 369, 371                   // 46
};

// https://github.com/multitheftauto/mtasa-blue/blob/55c5a42ee29a79554a3a09f3d19da5ced20ca742/Shared/sdk/SharedUtil.Game.hpp#L228
uint8_t GetPaletteIndexFromRGB(uint8_t from_r, uint8_t from_g, uint8_t from_b)
{
    unsigned long ulBestDist = 0xFFFFFFFF;
    uint8_t ucBestMatch = 0;
    for(unsigned int i = 0; i < std::size(paletteColorTable8) / 4; i++)
    {
        int r = paletteColorTable8[i * 4 + 0] - from_r;
        int g = paletteColorTable8[i * 4 + 1] - from_g;
        int b = paletteColorTable8[i * 4 + 2] - from_b;
        unsigned long ulDist = r * r + g * g + b * b;
        if(ulDist < ulBestDist)
        {
            ulBestDist = ulDist;
            ucBestMatch = i;
        }
    }
    return ucBestMatch;
}

uint16_t GetWeaponModel(int weaponid)
{
    if(weaponid < 0 || weaponid > 46) return 0;

    return g_usWeaponModels[weaponid];
}