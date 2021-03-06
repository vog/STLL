/*
 * STLL Simple Text Layouting Library
 *
 * STLL is the legal property of its developers, whose
 * names are listed in the COPYRIGHT file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef STLL_LAYOUTER_XML_SAVELOAD_H
#define STLL_LAYOUTER_XML_SAVELOAD_H

/* not part of the library: this file provides code to load and store layouts in XML files */

#include <stll/layouter.h>
#include <stll/layouterFont.h>

#include <pugixml.hpp>

/** \brief exception thrown when something went wrong with loading or saving
 */
class SaveLoadException_c : public std::runtime_error
{
  public:
    explicit SaveLoadException_c(const std::string & what_arg) : std::runtime_error(what_arg) {}

};

/** \brief load a layout from a XML node, the node must point to the "layout" node to load
 *  \param doc the XML node to load
 *  \param c the font cache to use to get the fonts from
 */
STLL::TextLayout_c loadLayoutFromXML(const pugi::xml_node & doc, std::shared_ptr<STLL::FontCache_c> c);

/** \brief store the layout into the given XML node. A new layout node is created and the date
 *  is inserted into that node
 *  \param l the layout to save
 *  \param doc the XML node to add the layout to
 *  \param c the cache that was used to create the fonts of the layout
 *  \attention this only works, when the fonts in the layout are all inside the cache
 *  and all the used fonts are created using file names (and not RAM pointers)
 */
void saveLayoutToXML(const STLL::TextLayout_c & l, pugi::xml_node & doc);

#endif
