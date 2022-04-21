#ifndef PROVINCE_PROPERTIES_PANE_H
# define PROVINCE_PROPERTIES_PANE_H

# include "WidgetContainer.h"

# include <set>
# include <string>

# include "gtkmm/scrolledwindow.h"
# include "gtkmm/checkbutton.h"
# include "gtkmm/comboboxtext.h"
# include "gtkmm/entry.h"
# include "gtkmm/box.h"

# include "Types.h"

# include "ProvincePreviewDrawingArea.h"

namespace MapNormalizer::GUI {
    /**
     * @brief The pane where properties of a province are placed into
     */
    class ProvincePropertiesPane: public WidgetContainer {
        public:
            ProvincePropertiesPane();

            Gtk::ScrolledWindow& getParent();

            void init();

            void setEnabled(bool = true);

            void setProvince(Province*, ProvincePreviewDrawingArea::DataPtr, bool = false);
            Province* getProvince();

            void onResize();

            void updateProperties(bool);

        protected:
            virtual void addWidgetToParent(Gtk::Widget&) override;

            void updateProperties(const Province*, bool);

            void rebuildContinentMenu(const std::set<std::string>&);

            void buildProvincePreviewView();

            void buildIsCoastalField();
            void buildProvinceTypeField();
            void buildTerrainTypeField();
            void buildContinentField();
            void buildStateCreationButton();

            void setPreview(ProvincePreviewDrawingArea::DataPtr);

        private:
            //! The province currently being acted upon
            Province* m_province;

            Gtk::Box m_box;
            Gtk::ScrolledWindow m_parent;

            ProvincePreviewDrawingArea m_preview_area;

            Gtk::CheckButton* m_is_coastal_button;
            Gtk::ComboBoxText* m_provtype_menu;
            Gtk::ComboBoxText* m_terrain_menu;
            Gtk::ComboBoxText* m_continent_menu;

            Gtk::Button* m_create_state_button;

            bool m_is_updating_properties;
    };
}

#endif

