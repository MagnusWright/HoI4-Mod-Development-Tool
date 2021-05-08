
#include "MapProject.h"

#include <fstream>
#include <cstring>
#include <cerrno>

#include "Logger.h"
#include "Constants.h"
#include "Util.h"

#include "ProvinceMapBuilder.h"

#include "HoI4Project.h"

MapNormalizer::Project::MapProject::MapProject(IProject& parent_project):
    m_shape_detection_info(),
    m_parent_project(parent_project)
{
}

MapNormalizer::Project::MapProject::~MapProject() {
}

/**
 * @brief Saves all map data
 *
 * @param path The root path of all map relatedd data
 *
 * @return True if all data was ablee to be successfully loaded, false otherwise
 */
bool MapNormalizer::Project::MapProject::save(const std::filesystem::path& path,
                                              std::error_code& ec)
{
    if(!std::filesystem::exists(path)) {
        writeDebug("Creating directory ", path);
        std::filesystem::create_directory(path);
    }

    if(m_shape_detection_info.provinces.empty()) {
        writeDebug("Nothing to write!");
        return true;
    }

    return saveShapeLabels(path, ec) && saveProvinceData(path, ec);
}

/**
 * @brief Loads all map data
 *
 * @param path The root path of all map related data
 *
 * @return True if all data was able to be successfully loaded, false otherwise
 */
bool MapNormalizer::Project::MapProject::load(const std::filesystem::path& path,
                                              std::error_code& ec)
{
    // If there is no root path for this subproject, then don't bother trying
    //  to load
    if(!std::filesystem::exists(path)) {
        // Do not set an error_code as there was no error, but return false
        //  because we are not actually loading any data
        return false;
    }

    // First we try to load the input map back up, as it holds important info
    //  about the map itself (such as dimensions, the original color value, etc...)
    auto inputs_root = dynamic_cast<HoI4Project&>(m_parent_project).getInputsRoot();
    auto input_provincemap_path = inputs_root / INPUT_PROVINCEMAP_FILENAME;
    if(!std::filesystem::exists(input_provincemap_path)) {
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
        writeWarning("Source import image does not exist, unable to finish loading data.");
        return false;
    } else {
        // Memory leak prevention
        if(m_shape_detection_info.image != nullptr) {
            delete m_shape_detection_info.image;
        }

        m_shape_detection_info.image = readBMP(input_provincemap_path);
        if(m_shape_detection_info.image == nullptr) {
            // TODO: We should instead pass ec into readBMP() and let it set ec
            //  to whatever might be appropriate
            ec = std::make_error_code(std::errc::io_error);
            writeWarning("Failed to read imported image.");
            return false;
        }
    }

    // Now load the other related data
    if(!loadProvinceData(path, ec) || !loadShapeLabels(path, ec)) {
        return false;
    }

    // Rebuild the graphics data
    auto* image = m_shape_detection_info.image;
    auto*& graphics_data = m_shape_detection_info.graphics_data;

    // Do this to prevent a memory leak
    if(graphics_data != nullptr) delete[] graphics_data;

    auto width = image->info_header.width;
    auto height = image->info_header.height;

    auto data_size = width * height * 3;
    graphics_data = new unsigned char[data_size];

    // Rebuild the graphics_data array
    for(uint32_t x = 0; x < width; ++x) {
        for(uint32_t y = 0; y < height; ++y) {
            // Get the index into the label matrix
            auto lindex = xyToIndex(image, x, y);

            // Get the index into the graphics data
            //  3 == the depth
            auto gindex = xyToIndex(image->info_header.width * 3, x * 3, y);

            auto label = m_shape_detection_info.label_matrix[lindex];

            // Error check
            if(label <= 0 || label > m_shape_detection_info.provinces.size()) {
                writeWarning("Label matrix has label ", label,
                             " at position (", x, ',', y, "), which is out of "
                             "the range of valid labels [1,",
                             m_shape_detection_info.provinces.size(), "]");
                continue;
            }

            auto& province = m_shape_detection_info.provinces[label - 1];

            graphics_data[gindex] = province.unique_color.r;
            graphics_data[gindex + 1] = province.unique_color.g;
            graphics_data[gindex + 2] = province.unique_color.b;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Writes all shape label data to a file.
 *
 * @param root The root where the shape label data should be written to
 *
 * @return True if the data was able to be successfully written, false otherwise.
 */
bool MapNormalizer::Project::MapProject::saveShapeLabels(const std::filesystem::path& root,
                                                         std::error_code& ec)
{
    auto path = root / SHAPEDATA_FILENAME;

    // write the shape finder data in a way that we can re-load it later
    if(std::ofstream out(path, std::ios::binary | std::ios::out); out)
    {
        out << SHAPEDATA_MAGIC;

        writeData(out, m_shape_detection_info.image->info_header.width, 
                       m_shape_detection_info.image->info_header.height);

        // Write the entire label matrix to the file
        out.write(reinterpret_cast<const char*>(m_shape_detection_info.label_matrix),
                  m_shape_detection_info.label_matrix_size * sizeof(uint32_t));
        out << '\0';
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        writeError("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Writes all province data to a .csv file (the same sort of file as
 *        would be loaded by HoI4
 *
 * @param root The root where the csv file should be written to
 *
 * @return True if the file was able to be successfully written, false otherwise.
 */
bool MapNormalizer::Project::MapProject::saveProvinceData(const std::filesystem::path& root,
                                                          std::error_code& ec)
{
    auto path = root / PROVINCEDATA_FILENAME;

    if(std::ofstream out(path); out) {
        // Write one line to the CSV for each province
        for(auto&& province : m_shape_detection_info.provinces) {
            
            out << province << std::endl;
        }
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        writeError("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Loads all shape label data out of $root/SHAPEDATA_FILENAME
 *
 * @param root The root where the shapedata is found
 *
 * @return True if the data was able to be loaded successfully, false otherwise
 */
bool MapNormalizer::Project::MapProject::loadShapeLabels(const std::filesystem::path& root,
                                                         std::error_code& ec)
{
    auto path = root / SHAPEDATA_FILENAME;

    if(!std::filesystem::exists(path)) {
        writeWarning("File ", path, " does not exist.");
        return false;
    } else if(std::ifstream in(path, std::ios::binary | std::ios::in); in) {
        unsigned char magic[4];
        uint32_t width = 0;
        uint32_t height = 0;

        // Read in all header information first, and make sure that we were 
        //  successful
        if(!safeRead(in, &magic, &width, &height)) {
            ec = std::error_code(static_cast<int>(errno), std::generic_category());
            writeError("Failed to read in header information. Reason: ", std::strerror(errno));
            return false;
        }

        auto& label_matrix_size = m_shape_detection_info.label_matrix_size;
        auto& label_matrix = m_shape_detection_info.label_matrix;

        if(label_matrix != nullptr) {
            delete[] label_matrix;
        }

        label_matrix_size = width * height;
        label_matrix = new uint32_t[label_matrix_size];

        if(!safeRead(label_matrix, label_matrix_size * sizeof(uint32_t), in)) {
            ec = std::error_code(static_cast<int>(errno), std::generic_category());
            writeError("Failed to read full label matrix. Reason: ", std::strerror(errno));

            delete[] label_matrix;
            label_matrix = nullptr;
            return false;
        }
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        writeError("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Load all province-level data from $root/PROVINCEDATA_FILENAME, the
 *        same type of .csv file loaded by HoI4.
 *
 * @param root The path to the root where the .csv file is
 *
 * @return True if the file was able to be successfully loaded, false otherwise.
 */
bool MapNormalizer::Project::MapProject::loadProvinceData(const std::filesystem::path& root,
                                                          std::error_code& ec)
{
    auto path = root / PROVINCEDATA_FILENAME;

    if(!std::filesystem::exists(path, ec)) {
        writeWarning("File ", path, " does not exist.");
        return false;
    } else if(std::ifstream in(path); in) {
        std::string line;

        // Make sure we don't have any provinces in the list first
        m_shape_detection_info.provinces.clear();

        // Get every line from the CSV file for parsing
        for(uint32_t line_num = 1; std::getline(in, line); ++line_num) {
            if(line.empty()) continue;

            std::stringstream ss(line);

            Province prov;

            // Attempt to parse the entire CSV line, we expect it to look like:
            //  ID;R;G;B;ProvinceType;IsCoastal;TerrainType;ContinentID
            if(!parseValues<';'>(ss, prov.id, prov.unique_color.r,
                                              prov.unique_color.g,
                                              prov.unique_color.b,
                                     prov.type, prov.coastal, prov.terrain,
                                     prov.continent))
            {
                ec = std::make_error_code(std::errc::bad_message);
                writeError("Failed to parse line #", line_num, ": '", line, "'");
                return false;
            }

            m_shape_detection_info.provinces.push_back(prov);
        }

        writeDebug("Loaded information for ",
                   m_shape_detection_info.provinces.size(), " provinces");
    } else {
        ec = std::error_code(static_cast<int>(errno), std::generic_category());
        writeError("Failed to open file ", path, ". Reason: ", std::strerror(errno));
        return false;
    }

    return true;
}

/**
 * @brief Loads data out of a ShapeFinder. We invalidate the original ShapeFinder
 *        as we want to take ownership of all pointers it holds
 *
 * @param shape_finder The ShapeFinder to load data from
 */
void MapNormalizer::Project::MapProject::setShapeFinder(ShapeFinder&& shape_finder)
{
    // We want to take ownership of all the internal data here
    //  TODO: Do we actually _need_ to do this?
    ShapeFinder sf(std::move(shape_finder));

    m_shape_detection_info.provinces = createProvincesFromShapeList(sf.getShapes());
    m_shape_detection_info.label_matrix = sf.getLabelMatrix();
    m_shape_detection_info.label_matrix_size = sf.getLabelMatrixSize();
    m_shape_detection_info.graphics_data = nullptr;
}

void MapNormalizer::Project::MapProject::setGraphicsData(unsigned char* data) {
    if(m_shape_detection_info.graphics_data != nullptr) {
        delete[] m_shape_detection_info.graphics_data;
    }

    m_shape_detection_info.graphics_data = data;
}

void MapNormalizer::Project::MapProject::setImage(BitMap* image) {
    m_shape_detection_info.image = image;
}

auto MapNormalizer::Project::MapProject::getImage() -> BitMap* {
    return m_shape_detection_info.image;
}

auto MapNormalizer::Project::MapProject::getImage() const -> const BitMap* {
    return m_shape_detection_info.image;
}

unsigned char* MapNormalizer::Project::MapProject::getGraphicsData() {
    return m_shape_detection_info.graphics_data;
}

const unsigned char* MapNormalizer::Project::MapProject::getGraphicsData() const
{
    return m_shape_detection_info.graphics_data;
}

