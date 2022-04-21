
#include "IMapDrawingArea.h"

#include "Logger.h"
#include "Constants.h"

#include "GraphicalDebugger.h"
#include "Util.h"

MapNormalizer::GUI::IMapDrawingAreaBase::IMapDrawingAreaBase():
    m_map_data(nullptr),
    m_on_select([](auto...) { }),      // The default callback does nothing
    m_on_multiselect([](auto...) { }),
    m_selections(),
    m_scale_factor(DEFAULT_ZOOM),
    m_viewing_mode(DEFAULT_VIEWING_MODE)
{ }


bool MapNormalizer::GUI::IMapDrawingAreaBase::hasData() const {
    return m_map_data != nullptr && !m_map_data->isClosed();
}

/**
 * @brief Changes the viewing mode and returns the old one
 *
 * @param viewing_mode The new viewing mode.
 *
 * @return The old viewing mode.
 */
auto MapNormalizer::GUI::IMapDrawingAreaBase::setViewingMode(ViewingMode viewing_mode)
    -> ViewingMode
{
    onViewingModeChange(viewing_mode);

    std::swap(m_viewing_mode, viewing_mode);

    return viewing_mode;
}

auto MapNormalizer::GUI::IMapDrawingAreaBase::getViewingMode() const
    -> ViewingMode
{
    return m_viewing_mode;
}

auto MapNormalizer::GUI::IMapDrawingAreaBase::getMapData() const
    -> std::shared_ptr<const MapData>
{
    return m_map_data;
}

void MapNormalizer::GUI::IMapDrawingAreaBase::setMapData(std::shared_ptr<const MapData> map_data)
{
    onSetData(map_data);

    m_map_data = map_data;

    resetZoom();
}

void MapNormalizer::GUI::IMapDrawingAreaBase::setOnProvinceSelectCallback(const SelectionCallback& callback)
{
    m_on_select = callback;
}

void MapNormalizer::GUI::IMapDrawingAreaBase::setOnMultiProvinceSelectionCallback(const SelectionCallback& callback)
{
    m_on_multiselect = callback;
}

void MapNormalizer::GUI::IMapDrawingAreaBase::setSelection() {
    onSelectionChanged(std::nullopt);
    m_selections.clear();
}

void MapNormalizer::GUI::IMapDrawingAreaBase::setSelection(const SelectionInfo& selection)
{
    onSelectionChanged(selection);
    m_selections = { selection };
}

void MapNormalizer::GUI::IMapDrawingAreaBase::addSelection(const SelectionInfo& selection)
{
    onSelectionChanged(selection);
    m_selections.insert(selection);
}

void MapNormalizer::GUI::IMapDrawingAreaBase::removeSelection(const SelectionInfo& selection)
{
    onSelectionChanged(selection);
    m_selections.erase(selection);
}

void MapNormalizer::GUI::IMapDrawingAreaBase::zoom(ZoomDirection direction) {
    switch(direction) {
        case ZoomDirection::IN:
            zoom(ZOOM_FACTOR);
            break;
        case ZoomDirection::OUT:
            zoom(-ZOOM_FACTOR);
            break;
        case ZoomDirection::RESET:
            resetZoom();
            break;
    }
}

void MapNormalizer::GUI::IMapDrawingAreaBase::zoom(double scale_factor_delta) {
    m_scale_factor += scale_factor_delta;

    onZoom();
}

/**
 * @brief Will reset the zoom level
 */
void MapNormalizer::GUI::IMapDrawingAreaBase::resetZoom() {
    // Do nothing if there is no data loaded
    if(!hasData()) return;

    auto* parent = getParent();

    if(parent == nullptr) {
        WRITE_WARN("MapDrawingArea has no parent, setting zoom to ", DEFAULT_ZOOM);

        m_scale_factor = DEFAULT_ZOOM;
    } else {
        double pwidth = parent->get_width();
        double pheight = parent->get_height();

        double iheight = m_map_data->getHeight();

        // Scale to the smallest dimension of the parent window
        // But only scale down if the image is too large, don't worry about
        //  trying to scale up a smaller image
        if(pheight <= pwidth && pheight < iheight) {
            m_scale_factor = (pheight / iheight);
        } else {
            m_scale_factor = DEFAULT_ZOOM;
        }
    }

    WRITE_DEBUG("Reset zoom to ", m_scale_factor);

    onZoom();
}

double MapNormalizer::GUI::IMapDrawingAreaBase::getScaleFactor() const {
    return m_scale_factor;
}

auto MapNormalizer::GUI::IMapDrawingAreaBase::getOnSelect() const
    -> const SelectionCallback&
{
    return m_on_select;
}

auto MapNormalizer::GUI::IMapDrawingAreaBase::getOnMultiSelect() const
    -> const SelectionCallback&
{
    return m_on_multiselect;
}

auto MapNormalizer::GUI::IMapDrawingAreaBase::getSelections() const
    -> const SelectionList&
{
    return m_selections;
}

auto MapNormalizer::GUI::IMapDrawingAreaBase::getSelections()
    -> SelectionList& 
{
    return m_selections;
}

std::ostream& MapNormalizer::GUI::operator<<(std::ostream& stream,
                                             const IMapDrawingAreaBase::ViewingMode& mode)
{
    switch(mode) {
        case IMapDrawingAreaBase::ViewingMode::PROVINCE_VIEW:
            stream << "PROVINCE_VIEW";
            break;
        case IMapDrawingAreaBase::ViewingMode::STATES_VIEW:
            stream << "STATES_VIEW";
            break;
    }

    return stream;
}

std::string std::to_string(const MapNormalizer::GUI::IMapDrawingAreaBase::ViewingMode& mode)
{
    std::stringstream ss;
    ss << mode;
    return ss.str();
}

