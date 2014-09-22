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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "layouterXHTML.h"

#include "utf-8.h"

#include <string>
#include <vector>

#include <boost/lexical_cast.hpp>

namespace STLL {

class indentShape_c : public shape_c
{
  private:
    const shape_c & outside;
    int32_t ind_left, ind_right;

  public:
    indentShape_c(const shape_c & s, int32_t li, int32_t ri) : outside(s), ind_left(li), ind_right(ri) { }

    virtual int32_t getLeft(int32_t top, int32_t bottom) const { return outside.getLeft(top, bottom)+ind_left; }
    virtual int32_t getRight(int32_t top, int32_t bottom) const { return outside.getRight(top, bottom)-ind_right; }
    virtual int32_t getLeft2(int32_t top, int32_t bottom) const { return outside.getLeft2(top, bottom)+ind_left; }
    virtual int32_t getRight2(int32_t top, int32_t bottom) const { return outside.getRight2(top, bottom)-ind_right; }
};

class shiftShape_c : public shape_c
{
  private:
    const shape_c & outside;
    int32_t shift;

  public:
    shiftShape_c(const shape_c & s, int32_t sh) : outside(s), shift(sh) { }

    virtual int32_t getLeft(int32_t top, int32_t bottom) const { return outside.getLeft(top+shift, bottom+shift); }
    virtual int32_t getRight(int32_t top, int32_t bottom) const { return outside.getRight(top+shift, bottom+shift); }
    virtual int32_t getLeft2(int32_t top, int32_t bottom) const { return outside.getLeft2(top+shift, bottom+shift); }
    virtual int32_t getRight2(int32_t top, int32_t bottom) const { return outside.getRight2(top+shift, bottom+shift); }
};

class stripLeftShape_c : public shape_c
{
  private:
    const shape_c & outside;
    int32_t ind_left, ind_right;

  public:
    stripLeftShape_c(const shape_c & s, int32_t li, int32_t ri) : outside(s), ind_left(li), ind_right(ri) { }

    virtual int32_t getLeft(int32_t top, int32_t bottom) const { return outside.getLeft(top, bottom)+ind_left; }
    virtual int32_t getRight(int32_t top, int32_t bottom) const { return outside.getLeft(top, bottom)+ind_right; }
    virtual int32_t getLeft2(int32_t top, int32_t bottom) const { return outside.getLeft2(top, bottom)+ind_left; }
    virtual int32_t getRight2(int32_t top, int32_t bottom) const { return outside.getLeft2(top, bottom)+ind_right; }
};

class stripRightShape_c : public shape_c
{
  private:
    const shape_c & outside;
    int32_t ind_left, ind_right;

  public:
    stripRightShape_c(const shape_c & s, int32_t li, int32_t ri) : outside(s), ind_left(li), ind_right(ri) { }

    virtual int32_t getLeft(int32_t top, int32_t bottom) const { return outside.getRight(top, bottom)-ind_left; }
    virtual int32_t getRight(int32_t top, int32_t bottom) const { return outside.getRight(top, bottom)-ind_right; }
    virtual int32_t getLeft2(int32_t top, int32_t bottom) const { return outside.getRight2(top, bottom)-ind_left; }
    virtual int32_t getRight2(int32_t top, int32_t bottom) const { return outside.getRight2(top, bottom)-ind_right; }
};

template <class T>
class vector2d {
  private:
    std::vector<std::vector<T> > data;
    T def;
  public:
    void set(size_t x, size_t y, const T & val) {
      if (data.size() <= y) data.resize(y+1);
      if (data[y].size() <= x) data[y].resize(x+1);
      data[y][x] = val;
    }

    const T & get(size_t x, size_t y) const
    {
      if (y >= data.size() || x >= data[y].size()) return def;
      return data[y][x];
    }

    T & get(size_t x, size_t y)
    {
      if (y >= data.size() || x >= data[y].size()) return def;
      return data[y][x];
    }

    void rectangularize(void)
    {
      data.resize(data.size()+1);
      size_t m = 0;

      for (const auto & a : data)
        m = std::max(m, a.size());

      m++;

      for (auto & a : data)
        a.resize(m);
    }
};

static uint8_t hex2num(char c)
{
  switch (c)
  {
    case '0': return 0;
    case '1': return 1;
    case '2': return 2;
    case '3': return 3;
    case '4': return 4;
    case '5': return 5;
    case '6': return 6;
    case '7': return 7;
    case '8': return 8;
    case '9': return 9;
    case 'a':
    case 'A': return 10;
    case 'b':
    case 'B': return 11;
    case 'c':
    case 'C': return 12;
    case 'd':
    case 'D': return 13;
    case 'e':
    case 'E': return 14;
    case 'f':
    case 'F': return 15;
    default:
      throw XhtmlException_c("Wrong format for a hex-number");
  }
}

static uint8_t hex2byte(char c1, char c2)
{
  return hex2num(c1)* 16 + hex2num(c2);
}

/** \brief evaluate a color string
 *  \param col string with the color
 *  \param r red value of the color
 *  \param g green value of the color
 *  \param b blue value of the color
 */
static color_c evalColor(const std::string & col)
{
  if (col == "transparent")
  {
    return color_c();
  }
  else
  {
    return color_c(hex2byte(col[1], col[2]), hex2byte(col[3], col[4]), hex2byte(col[5], col[6]));
  }
}

class szFunctor
{
  public:
    virtual double get(void) = 0;
};


/** \brief evaluate size
 *  \param sz the size string from the CSS
 *  \return the resulting size in pixel
 */
static double evalSize(const std::string & sz, szFunctor * f = nullptr)
{
  // right now we accept only pixel sizes
  size_t l = sz.length();

  if (sz[l-2] == 'p' && sz[l-1] == 'x')
  {
    return 64*atof(sz.c_str());
  }
  else if (f && sz[l-1] == '%')
  {
    return f->get() * atof(sz.c_str()) / 100;
  }

  throw XhtmlException_c("only pixel size format is supported");

  return 0;
}

class parentFunctor : public szFunctor
{
  private:
    std::string tag;
    pugi::xml_node node;
    const textStyleSheet_c & rules;

  public:
    parentFunctor(const std::string & t, pugi::xml_node n, const textStyleSheet_c & r) :
       tag(t), node(n), rules(r) {}

    double get(void)
    {
      if (!node)
        throw XhtmlException_c("no parent node to base a percent value on");

      parentFunctor f(tag, node.parent(), rules);
      return evalSize(rules.getValue(node, tag), &f);
    }
};

static std::vector<codepointAttributes::shadow> evalShadows(const std::string & v)
{
  std::vector<codepointAttributes::shadow> s;

  codepointAttributes::shadow sh;

  size_t spos = 0;

  while (spos < v.length())
  {
    while (v[spos] == ' ' && spos < v.length()) spos++;
    if (spos >= v.length()) throw XhtmlException_c("Format of shadow invalid");

    sh.dx = evalSize(v.substr(spos, v.find(' ', spos)-spos));

    while (v[spos] != ' ' && spos < v.length()) spos++;
    if (spos >= v.length()) throw XhtmlException_c("Format of shadow invalid");

    while (v[spos] == ' ' && spos < v.length()) spos++;
    if (spos >= v.length()) throw XhtmlException_c("Format of shadow invalid");

    sh.dy = evalSize(v.substr(spos, v.find(' ', spos)-spos));

    while (v[spos] != ' ' && spos < v.length()) spos++;
    if (spos >= v.length()) throw XhtmlException_c("Format of shadow invalid");

    while (v[spos] == ' ' && spos < v.length()) spos++;
    if (spos >= v.length()) throw XhtmlException_c("Format of shadow invalid");

    sh.c = evalColor(v.substr(spos, v.find(',', spos)-spos));

    s.push_back(sh);

    while (v[spos] != ',' && spos < v.length()) spos++;
    if (spos >= v.length()) break;

    spos++;
  }

  return s;
}

static std::string normalizeHTML(const std::string & in, char prev)
{
  std::string out;

  for (auto a : in)
  {
    if (a == '\n' || a == '\r')
      a = ' ';

    if (a != ' ' || prev != ' ')
      out += a;

    prev = a;
  }

  return out;
}

static std::string getNodePath(const pugi::xml_node & xml)
{
  if (xml.empty())
    return "";
  else
    return getNodePath(xml.parent()) + "/" + xml.name();
}

static std::shared_ptr<fontFace_c> getFontForNode(const pugi::xml_node & xml, const textStyleSheet_c & rules)
{
  std::string fontFamily = rules.getValue(xml, "font-family");
  std::string fontStyle = rules.getValue(xml, "font-style");
  std::string fontVariant = rules.getValue(xml, "font-variant");
  std::string fontWeight = rules.getValue(xml, "font-weight");

  parentFunctor fkt("font-size", xml.parent(), rules);
  double fontSize = evalSize(rules.getValue(xml, "font-size"), &fkt);

  auto f = rules.findFamily(fontFamily)->getFont(fontSize, fontStyle, fontVariant, fontWeight);

  if (!f)
  {
    throw XhtmlException_c(std::string("Requested font not found (family:'") + fontFamily +
                                       "', style: '" + fontStyle +
                                       "', variant: '" + fontVariant +
                                       "', weight: '" + fontWeight + ") required here: " + getNodePath(xml));
  }

  return f;
}

static const pugi::char_t * getHTMLAttribute(pugi::xml_node xml, const std::string attr)
{
  while (true)
  {
    const pugi::char_t * res = xml.attribute(attr.c_str()).value();

    if (res && *res) return res;

    if ("lang" == attr)
    {
      xml = xml.parent();

      if (xml.empty())
      {
        return res;
      }
    }
    else
    {
      return res;
    }
  }
}

typedef textLayout_c (*ParseFunction)(pugi::xml_node & xml, const textStyleSheet_c & rules,
                                      const shape_c & shape, int32_t ystart);


// handles padding, margin and border, all in one, it takes the text returned from the
// ParseFunction and boxes it
static textLayout_c boxIt(const pugi::xml_node & xml, pugi::xml_node & xml2, const textStyleSheet_c & rules,
                          const shape_c & shape, int32_t ystart, ParseFunction fkt,
                          const pugi::xml_node & above, const pugi::xml_node & left,
                          bool collapseBorder = false, uint32_t minHeight = 0)
{
  int32_t padding_left = 0;
  int32_t padding_right = 0;
  int32_t padding_top = 0;
  int32_t padding_bottom = 0;

  padding_left = padding_right = padding_top = padding_bottom = evalSize(rules.getValue(xml, "padding"));

  if (rules.getValue(xml, "padding-left") != "")   padding_left   = evalSize(rules.getValue(xml, "padding-left"));
  if (rules.getValue(xml, "padding-right") != "")  padding_right  = evalSize(rules.getValue(xml, "padding-right"));
  if (rules.getValue(xml, "padding-top") != "")    padding_top    = evalSize(rules.getValue(xml, "padding-top"));
  if (rules.getValue(xml, "padding-bottom") != "") padding_bottom = evalSize(rules.getValue(xml, "padding-bottom"));

  int32_t borderwidth_left = 0;
  int32_t borderwidth_right = 0;
  int32_t borderwidth_top = 0;
  int32_t borderwidth_bottom = 0;

  borderwidth_left = borderwidth_right = borderwidth_top = borderwidth_bottom = evalSize(rules.getValue(xml, "border-width"));

  if (rules.getValue(xml, "border-left-width") != "")   borderwidth_left   = evalSize(rules.getValue(xml, "border-left-width"));
  if (rules.getValue(xml, "border-right-width") != "")  borderwidth_right  = evalSize(rules.getValue(xml, "border-right-width"));
  if (rules.getValue(xml, "border-top-width") != "")    borderwidth_top    = evalSize(rules.getValue(xml, "border-top-width"));
  if (rules.getValue(xml, "border-bottom-width") != "") borderwidth_bottom = evalSize(rules.getValue(xml, "border-bottom-width"));

  int32_t margin_left = 0;
  int32_t margin_right = 0;
  int32_t margin_top = 0;
  int32_t margin_bottom = 0;

  margin_left = margin_right = margin_top = margin_bottom = evalSize(rules.getValue(xml, "margin"));

  if (rules.getValue(xml, "margin-left") != "")   margin_left   = evalSize(rules.getValue(xml, "margin-left"));
  if (rules.getValue(xml, "margin-right") != "")  margin_right  = evalSize(rules.getValue(xml, "margin-right"));
  if (rules.getValue(xml, "margin-top") != "")    margin_top    = evalSize(rules.getValue(xml, "margin-top"));
  if (rules.getValue(xml, "margin-bottom") != "") margin_bottom = evalSize(rules.getValue(xml, "margin-bottom"));

  int32_t marginElementAbove = 0;
  int32_t marginElementLeft = 0;
  int32_t borderElementAbove = 0;
  int32_t borderElementLeft = 0;

  if (!above.empty())
  {
    marginElementAbove = evalSize(rules.getValue(above, "margin"));
    if (rules.getValue(above, "margin-bottom") != "")
      marginElementAbove = evalSize(rules.getValue(above, "margin-bottom"));

    if (margin_top == 0 && marginElementAbove == 0)
    {
      borderElementAbove = evalSize(rules.getValue(above, "border-width"));
      if (rules.getValue(above, "border-bottom-width") != "")
        borderElementAbove = evalSize(rules.getValue(above, "border-bottom-width"));
    }
  }

  if (!left.empty())
  {
    marginElementLeft = evalSize(rules.getValue(left, "margin"));
    if (rules.getValue(left, "margin-right") != "")
      marginElementLeft = evalSize(rules.getValue(left, "margin-right"));

    if (margin_left == 0 && marginElementLeft == 0)
    {
      borderElementLeft = evalSize(rules.getValue(left, "border-width"));
      if (rules.getValue(left, "border-right-width") != "")
        borderElementLeft = evalSize(rules.getValue(left, "border-right-width"));
    }
  }

  margin_top = std::max(marginElementAbove, margin_top)-marginElementAbove;
  margin_left = std::max(marginElementLeft, margin_left)-marginElementLeft;

  if (collapseBorder)
  {
    borderwidth_top =  std::max(borderElementAbove, borderwidth_top)-borderElementAbove;
    borderwidth_left = std::max(borderElementLeft, borderwidth_left)-borderElementLeft;
  }

  auto l2 = fkt(xml2, rules,
                indentShape_c(shape, padding_left+borderwidth_left+margin_left, padding_right+borderwidth_right+margin_right),
                ystart+padding_top+borderwidth_top+margin_top);
  l2.setHeight(std::max(minHeight, l2.getHeight()+padding_bottom+borderwidth_bottom+margin_bottom));

  textLayout_c::commandData c;

  c.command = textLayout_c::commandData::CMD_RECT;

  if (borderwidth_top)
  {
    std::string color = rules.getValue(xml, "border-color");
    if (rules.getValue(xml, "border-top-color") != "") color = rules.getValue(xml, "border-top-color");
    if (color == "")                                   color = rules.getValue(xml, "color");
    c.c = evalColor(color);

    if (c.c.a() != 0)
    {
      c.x = l2.getLeft()-padding_left-borderwidth_left;
      c.y = ystart+margin_top;
      c.w = l2.getRight()-l2.getLeft()+padding_left+padding_right+borderwidth_left+borderwidth_right;
      c.h = borderwidth_top;
      l2.addCommandStart(c);
    }
  }

  if (borderwidth_bottom)
  {
    std::string color = rules.getValue(xml, "border-color");
    if (rules.getValue(xml, "border-bottom-color") != "") color = rules.getValue(xml, "border-bottom-color");
    if (color == "")                                      color = rules.getValue(xml, "color");
    c.c = evalColor(color);

    if (c.c.a() != 0)
    {
      c.x = l2.getLeft()-padding_left-borderwidth_left;
      c.y = l2.getHeight()-borderwidth_bottom-margin_bottom;
      c.w = l2.getRight()-l2.getLeft()+padding_left+padding_right+borderwidth_left+borderwidth_right;
      c.h = borderwidth_bottom;
      l2.addCommandStart(c);
    }
  }

  if (borderwidth_right)
  {
    std::string color = rules.getValue(xml, "border-color");
    if (rules.getValue(xml, "border-right-color") != "") color = rules.getValue(xml, "border-right-color");
    if (color == "")                                     color = rules.getValue(xml, "color");
    c.c = evalColor(color);

    if (c.c.a() != 0)
    {
      c.x = l2.getRight()+padding_right;
      c.y = ystart+margin_top;
      c.w = borderwidth_right;
      c.h = l2.getHeight()-ystart-margin_bottom-margin_top;
      l2.addCommandStart(c);
    }
  }

  if (borderwidth_left)
  {
    std::string color = rules.getValue(xml, "border-color");
    if (rules.getValue(xml, "border-left-color") != "") color = rules.getValue(xml, "border-left-color");
    if (color == "")                                    color = rules.getValue(xml, "color");
    c.c = evalColor(color);

    if (c.c.a() != 0)
    {
      c.x = l2.getLeft()-padding_left-borderwidth_left;
      c.y = ystart+margin_top;
      c.w = borderwidth_left;
      c.h = l2.getHeight()-ystart-margin_bottom-margin_top;
      l2.addCommandStart(c);
    }
  }

  c.c = evalColor(rules.getValue(xml, "background-color"));

  if (c.c.a() != 0)
  {
    c.command = textLayout_c::commandData::CMD_RECT;

    c.x = shape.getLeft(ystart+margin_top, ystart+margin_top)+borderwidth_left+margin_left;
    c.y = ystart+borderwidth_top+margin_top;
    c.w = shape.getRight(ystart+margin_top, ystart+margin_top)-
          shape.getLeft(ystart+margin_top, ystart+margin_top)-borderwidth_right-borderwidth_left-margin_right-margin_left;
    c.h = l2.getHeight()-ystart-borderwidth_bottom-borderwidth_top-margin_bottom-margin_top;
    l2.addCommandStart(c);
  }

#ifdef _DEBUG_ // allows to see the boxes using a random color for each

  textLayout_c::commandData c;
  c.command = textLayout_c::commandData::CMD_RECT;
  c.x = shape.getLeft(ystart, ystart);
  c.y = ystart;
  c.w = shape.getRight(ystart, ystart)-shape.getLeft(ystart, ystart);
  c.h = l2.getHeight()-ystart;
  c.r = rand() % 128;
  c.g = rand() % 128;
  c.b = rand() % 128;
  c.a = 128;

  l2.addCommandStart(c);

#endif

  l2.setLeft(l2.getLeft()-padding_left-borderwidth_left-margin_left);
  l2.setRight(l2.getRight()+padding_right+borderwidth_right+margin_right);

  return l2;
}

static textLayout_c layoutXML_IMG(pugi::xml_node & xml, const textStyleSheet_c & rules,
                                  const shape_c & shape, int32_t ystart)
{
  textLayout_c l;

  textLayout_c::commandData c;
  c.command = textLayout_c::commandData::CMD_IMAGE;
  c.x = shape.getLeft(ystart, ystart);
  c.y = ystart;
  c.imageURL = xml.attribute("src").value();
  l.addCommand(c);
  l.setHeight(ystart+evalSize(xml.attribute("height").value()));
  l.setLeft(c.x);
  l.setRight(c.x+evalSize(xml.attribute("width").value()));

  return l;
}

// this function is different from all the other layout functions as it
// will take the first XML-node to layout and work along with the siblings
// instead of looking at the children
// this function will also return a new node where it stopped working
pugi::xml_node layoutXML_text(pugi::xml_node xml, const textStyleSheet_c & rules, std::u32string & txt,
               attributeIndex_c & attr, int32_t baseline = 0)
{
  while (xml)
  {
    if (xml.type() == pugi::node_pcdata)
    {
      size_t s = txt.length();

      if (txt.length() == 0)
        txt = u8_convertToU32(normalizeHTML(xml.value(), ' '));
      else
        txt += u8_convertToU32(normalizeHTML(xml.value(), txt[txt.length()-1]));

      codepointAttributes a;

      a.c = evalColor(rules.getValue(xml.parent(), "color"));
      a.font = getFontForNode(xml.parent(), rules);
      a.lang = getHTMLAttribute(xml.parent(), "lang");
      a.flags = 0;
      if (rules.getValue(xml.parent(), "text-decoration") == "underline")
      {
        a.flags |= codepointAttributes::FL_UNDERLINE;
      }
      a.shadows = evalShadows(rules.getValue(xml.parent(), "text-shadow"));

      a.baseline_shift = baseline;

      attr.set(s, txt.length()-1, a);
    }
    else if (   (xml.type() == pugi::node_element)
             && (   (std::string("i") == xml.name())
                 || (std::string("span") == xml.name())
                )
            )
    {
      if (layoutXML_text(xml.first_child(), rules, txt, attr))
      {
        throw XhtmlException_c("something unexpected happend in a phrasing segment (" + getNodePath(xml) + ")");
      }
    }
    else if (   (xml.type() == pugi::node_element)
             && (std::string("sub") == xml.name())
            )
    {
      auto font = getFontForNode(xml, rules);

      if (layoutXML_text(xml.first_child(), rules, txt, attr, baseline-font->getAscender()/2))
      {
        throw XhtmlException_c("something unexpected happend in a phrasing segment (" + getNodePath(xml) + ")");
      }
    }
    else if (   (xml.type() == pugi::node_element)
             && (std::string("sup") == xml.name())
            )
    {
      auto font = getFontForNode(xml.parent(), rules);

      layoutXML_text(xml, rules, txt, attr, baseline+font->getAscender()/2);
    }
    else if ((xml.type() == pugi::node_element) && (std::string("br") == xml.name()))
    {
      txt += U'\n';
      codepointAttributes a;
      a.flags = 0;
      a.font = getFontForNode(xml.parent(), rules);
      a.lang = getHTMLAttribute(xml.parent(), "lang");
      attr.set(txt.length()-1, a);
    }
    else if ((xml.type() == pugi::node_element) && (std::string("img") == xml.name()))
    {
      codepointAttributes a;
      a.inlay = std::make_shared<textLayout_c>(boxIt(xml, xml, rules, rectangleShape_c(10000), 0,
                                                     layoutXML_IMG, pugi::xml_node(), pugi::xml_node()));
      a.baseline_shift = 0;
      a.shadows = evalShadows(rules.getValue(xml.parent(), "text-shadow"));

      // if we want underlines, we add the font so that the layouter
      // can find the position of the underline
      if (rules.getValue(xml.parent(), "text-decoration") == "underline")
      {
        a.flags |= codepointAttributes::FL_UNDERLINE;
        a.font = getFontForNode(xml.parent(), rules);
        a.c = evalColor(rules.getValue(xml.parent(), "color"));
      }
      txt += U'\u00A0';
      attr.set(txt.length()-1, a);
    }
    else
    {
      break;
    }

    xml = xml.next_sibling();
  }
  return xml;
}

// this function is different from all other layout functions usable in the boxIt
// function, as it will change the xml node and return a new one
static textLayout_c layoutXML_Phrasing(pugi::xml_node & xml, const textStyleSheet_c & rules, const shape_c & shape, int32_t ystart)
{
  std::u32string txt;
  attributeIndex_c attr;

  auto xml2 = layoutXML_text(xml, rules, txt, attr);

  layoutProperties lprop;
  std::string s = rules.getValue(xml, "text-align");

  if      (s == "left")   lprop.align = layoutProperties::ALG_LEFT;
  else if (s == "right")  lprop.align = layoutProperties::ALG_RIGHT;
  else if (s == "center") lprop.align = layoutProperties::ALG_CENTER;
  else if (s == "justify") {
    s = rules.getValue(xml, "text-align-last");
    if      (s == "left")  lprop.align = layoutProperties::ALG_JUSTIFY_LEFT;
    else if (s == "right") lprop.align = layoutProperties::ALG_JUSTIFY_RIGHT;
    else if (s == "")
    {
      s = rules.getValue(xml, "direction");
      if (s == "ltr") lprop.align = layoutProperties::ALG_JUSTIFY_LEFT;
      if (s == "rtl") lprop.align = layoutProperties::ALG_JUSTIFY_RIGHT;
    }
    else
    {
      throw XhtmlException_c("Only 'left' and 'right' are allowed as values for the "
                             "'text-align-last' CSS property (" + getNodePath(xml) + ")");
    }
  }
  else if (s == "")
  {
    s = rules.getValue(xml, "direction");
    if (s == "ltr") lprop.align = layoutProperties::ALG_LEFT;
    if (s == "rtl") lprop.align = layoutProperties::ALG_RIGHT;
  }
  else
  {
    throw XhtmlException_c("Only 'left, 'right', 'center' and 'justify' are allowed for "
                           "the 'text-align' CSS property (" + getNodePath(xml) + ")");
  }

  lprop.indent = evalSize(rules.getValue(xml, "text-indent"));
  lprop.ltr = rules.getValue(xml, "direction") == "ltr";
  lprop.round = rules.getRound();

  xml = xml2;

  return layoutParagraph(txt, attr, shape, lprop, ystart);
}

static textLayout_c layoutXML_Flow(pugi::xml_node & txt, const textStyleSheet_c & rules, const shape_c & shape, int32_t ystart);

static textLayout_c layoutXML_UL(pugi::xml_node & xml, const textStyleSheet_c & rules, const shape_c & shape, int32_t ystart)
{
  textLayout_c l;
  l.setHeight(ystart);
  for (auto & i : xml)
  {
    if (   (i.type() == pugi::node_element)
        && (std::string("li") == i.name())
       )
    {
      pugi::xml_node j = xml;
      while (j.type() != pugi::node_pcdata)
        j = j.first_child();

      auto font = getFontForNode(j, rules);
      auto y = l.getHeight();

      codepointAttributes a;
      a.c = evalColor(rules.getValue(xml, "color"));
      a.font = font;
      a.lang = "";
      a.flags = 0;
      a.shadows = evalShadows(rules.getValue(xml, "text-shadow"));

      int32_t padding = evalSize(rules.getValue(i, "padding"));
      int32_t listIndent = font->getAscender();

      auto direction = rules.getValue(xml, "direction");

      layoutProperties prop;
      prop.indent = 0;
      prop.ltr = true;
      prop.align = layoutProperties::ALG_CENTER;
      prop.round = rules.getRound();

      if (direction == "ltr")
      {
        l.append(layoutParagraph(U"\u2022", attributeIndex_c(a),
                                 stripLeftShape_c(shape, padding, padding+listIndent), prop, y+padding));
        l.append(boxIt(i, i, rules, indentShape_c(shape, listIndent, 0), y, layoutXML_Flow, i.previous_sibling(), pugi::xml_node()));
      }
      else
      {
        l.append(layoutParagraph(U"\u2022", attributeIndex_c(a),
                                 stripRightShape_c(shape, padding+listIndent, padding), prop, y+padding));
        l.append(boxIt(i, i, rules, indentShape_c(shape, 0, listIndent), y, layoutXML_Flow, i.previous_sibling(), pugi::xml_node()));
      }

      l.setLeft(shape.getLeft2(ystart, l.getHeight()));
      l.setRight(shape.getRight2(ystart, l.getHeight()));
    }
    else
    {
      throw XhtmlException_c("Only 'li' tags allowed within 'ul' tag (" + getNodePath(i) + ")");
    }
  }

  return l;
}

typedef struct
{
  uint32_t row;
  uint32_t col;
  uint32_t rowspan;
  uint32_t colspan;

  pugi::xml_node xml;

  textLayout_c l;

} tableCell;

static void layoutXML_TR(pugi::xml_node & xml, uint32_t row, const textStyleSheet_c & /* rules */,
                         std::vector<tableCell> & cells, vector2d<pugi::xml_node> & cellarray)
{
  uint32_t col = 0;
  while (!cellarray.get(col+1, row+1).empty()) col++;

  // collect all cells of the row
  for (const auto & i : xml)
  {
    if (   (i.type() == pugi::node_element)
        && (   (std::string("th") == i.name())
            || (std::string("td") == i.name())
           )
       )
    {
      tableCell c;

      c.xml = i;
      c.rowspan = 1;
      c.colspan = 1;

      for (const auto & a : i.attributes())
      {
        if (std::string("rowspan") == a.name())
        {
          c.rowspan = std::stoi(a.value());
        }
        if (std::string("colspan") == a.name())
        {
          c.colspan = std::stoi(a.value());
        }
      }

      c.row = row;
      c.col = col;

      cells.emplace_back(c);

      for (uint32_t x = col; x < col+c.colspan; x++)
        for (uint32_t y = row; y < row+c.rowspan; y++)
          cellarray.set(x+1, y+1, i);

      col += c.colspan;
    }
    else
    {
      throw XhtmlException_c("Only 'th' or 'td' tags allowed within 'ul' tag (" + getNodePath(i) + ")");
    }
  }
}

static textLayout_c layoutXML_TABLE(pugi::xml_node & xml, const textStyleSheet_c & rules, const shape_c & shape, int32_t ystart)
{
  std::vector<tableCell> cells;
  std::vector<uint32_t> widths;
  std::vector<uint32_t> colStart;
  vector2d<pugi::xml_node> cellarray;
  uint32_t row = 0;
  bool col = false;
  bool rtl = (rules.getValue(xml, "direction") == "rtl");
  int left = rtl ? 1 : -1;

  // collect all cells of the table
  for (auto & i : xml)
  {
    if (   (i.type() == pugi::node_element)
        && (std::string("colgroup") == i.name())
       )
    {
      for (const auto & j : i)
      {
        if (   (j.type() == pugi::node_element)
            && (std::string("col") == j.name())
           )
        {
          int span = 1;
          auto a = j.attribute("span");
          if (a)
          {
            span = std::stoi(a.value());
          }

          if (span == 0)
          {
            throw XhtmlException_c("malformed 'span' attribute (" + getNodePath(j) + ")");
          }

          uint32_t width = evalSize(rules.getValue(j, "width"));

          while (span > 0)
          {
            widths.push_back(width);
            span--;
          }
        }
        else
        {
          throw XhtmlException_c("Only 'col' tags allowed within 'colgroup' tag (" + getNodePath(j) + ")");
        }
      }
      col = true;
    }
    else if (   (i.type() == pugi::node_element)
        && (std::string("tr") == i.name())
       )
    {
      if (!col)
      {
        throw XhtmlException_c("You must define columns and widths in a table (" + getNodePath(i) + ")");
      }

      layoutXML_TR(i, row, rules, cells, cellarray);
      row++;
    }
    else
    {
      throw XhtmlException_c("Only 'tr' and 'colgroup' tags allowed within 'table' tag (" + getNodePath(i) + ")");
    }
  }

  // make cellarray contain one extra row and column to get the neighbors easily
  cellarray.rectangularize();

  // calculate the start of each columns
  colStart.push_back(0);
  for (size_t i = 0; i < widths.size(); i++)
    colStart.push_back(*colStart.rbegin() + widths[i]);

  // make a first run over all cells to get a first minimal
  // hight for each cell
  for (auto & c : cells)
  {
    c.l = boxIt(c.xml, c.xml, rules, rectangleShape_c(colStart[c.col+c.colspan]-colStart[c.col]),
                0, layoutXML_Flow, cellarray.get(c.col+1, c.row), cellarray.get(c.col+(1+left)*c.colspan, c.row+1),
                rules.getValue(xml, "border-collapse") == "collapse");
  }

  // calculate the height of each row of the table by finding the cell with the maximal
  // height for each row
  // multi row cells only count for the final row
  uint32_t maxrow = 0;

  for (auto & c : cells)
  {
    maxrow = std::max(maxrow, c.row+c.rowspan);
  }

  std::vector<uint32_t> rowheights(maxrow);

  for (auto & c : cells)
  {
    if (c.rowspan == 1)
    {
      rowheights[c.row] = std::max(rowheights[c.row], c.l.getHeight());
    }
  }

  for (auto & c : cells)
  {
    if (c.rowspan > 1)
    {
      uint32_t h = 0;

      for (size_t r = c.row; r < c.row+c.rowspan; r++)
        h += rowheights[r];

      if (h < c.l.getHeight())
        rowheights[c.row+c.rowspan-1] += c.l.getHeight()-h;
    }
  }

  // find out the real width of the table
  int width = *colStart.rbegin();

  // place table into the center... TODO we may not always want to do that
  int xindent = 0;

  xindent = shape.getLeft(ystart, ystart) + (shape.getRight(ystart, ystart) - shape.getLeft(ystart, ystart) - width) / 2 ;

  if (xindent < 0) xindent = 0;

  // layout the table
  textLayout_c l;
  row = 0;

  for (auto & c : cells)
  {
    if (row != c.row)
    {
      ystart += rowheights[row];
      row = c.row;
    }

    uint32_t rh = 0;
    for (size_t r = row; r < row+c.rowspan; r++)
      rh += rowheights[r];

    if (rh != c.l.getHeight())
      c.l = boxIt(c.xml, c.xml, rules, rectangleShape_c(colStart[c.col+c.colspan]-colStart[c.col]),
                  0, layoutXML_Flow, cellarray.get(c.col+1, c.row), cellarray.get(c.col+(1+left)*c.colspan, c.row+1),
                  rules.getValue(xml, "border-collapse") == "collapse", rh);

    if (rtl)
    {
      l.addCommandVector(c.l.data, xindent+*colStart.rbegin()-colStart[1]+colStart[0]-colStart[c.col+c.colspan-1], ystart);
    }
    else
    {
      l.addCommandVector(c.l.data, colStart[c.col]+xindent, ystart);
    }
  }

  l.setHeight(ystart+rowheights[row]);
  l.setLeft(xindent+colStart[0]);
  l.setRight(xindent+width);

  return l;
}

static textLayout_c layoutXML_Flow(pugi::xml_node & txt, const textStyleSheet_c & rules, const shape_c & shape, int32_t ystart)
{
  textLayout_c l;
  l.setHeight(ystart);

  auto i = txt.first_child();

  while (i)
  {
    if (   (i.type() == pugi::node_element)
        && (   (std::string("p") == i.name())
            || (std::string("h1") == i.name())
            || (std::string("h2") == i.name())
            || (std::string("h3") == i.name())
            || (std::string("h4") == i.name())
            || (std::string("h5") == i.name())
            || (std::string("h6") == i.name())
           )
       )
    {
      // these element start a phrasing context
      auto j = i.first_child();
      l.append(boxIt(i, j, rules, shape, l.getHeight(), layoutXML_Phrasing, i.previous_sibling(), pugi::xml_node()));
      if (j)
      {
        throw XhtmlException_c("There was an unexpected tag within a phrasing context (" + getNodePath(i) + ")");
      }
      i = i.next_sibling();
    }
    else if (  (i.type() == pugi::node_pcdata )
             ||(  (i.type() == pugi::node_element)
                &&(std::string("span") == i.name())
               )
            )
    {
      // these elements make the current node into a phrasing node
      // after parsing, we assume right now, i will be changed to point to the next node
      // not taken up by the Phrasing environment, so we don't want
      // i to be set to the next sibling as in all other cases
      l.append(layoutXML_Phrasing(i, rules, shape, l.getHeight()));
    }
    else if (i.type() == pugi::node_element && std::string("table") == i.name())
    {
      l.append(boxIt(i, i, rules, shape, l.getHeight(), layoutXML_TABLE, i.previous_sibling(), pugi::xml_node()));
      i = i.next_sibling();
    }
    else if (i.type() == pugi::node_element && std::string("ul") == i.name())
    {
      l.append(boxIt(i, i, rules, shape, l.getHeight(), layoutXML_UL, i.previous_sibling(), pugi::xml_node()));
      i = i.next_sibling();
    }
    else if (i.type() == pugi::node_element && std::string("div") == i.name())
    {
      l.append(boxIt(i, i, rules, shape, l.getHeight(), layoutXML_Flow, i.previous_sibling(), pugi::xml_node()));
      i = i.next_sibling();
    }
    else
    {
      throw XhtmlException_c("Only 'p', 'h1'-'h6', 'ul' and 'table' tag and prasing context is "
                             "is allowed within flow environment (" + getNodePath(i) + ")");
    }
  }

  l.setLeft(shape.getLeft(ystart, l.getHeight()));
  l.setRight(shape.getRight(ystart, l.getHeight()));

  return l;
}

static textLayout_c layoutXML_HTML(const pugi::xml_node & txt, const textStyleSheet_c & rules, const shape_c & shape)
{
  textLayout_c l;

  bool headfound = false;
  bool bodyfound = false;

  for (auto & i : txt)
  {
    if (i.type() == pugi::node_element && std::string("head") == i.name() && !headfound)
    {
      headfound = true;
    }
    else if (i.type() == pugi::node_element && std::string("body") == i.name() && !bodyfound)
    {
      bodyfound = true;
      l = boxIt(i, i, rules, shape, 0, layoutXML_Flow, i.previous_sibling(), pugi::xml_node());
    }
    else
    {
      throw XhtmlException_c("Only up to one 'head' and up to one 'body' tag and no other "
                             "tags are allowed inside the 'html' tag (" + getNodePath(i) + ")");
    }
  }

  return l;
}

textLayout_c layoutXML(const pugi::xml_document & txt, const textStyleSheet_c & rules, const shape_c & shape)
{
  textLayout_c l;

  // we must have a HTML root node
  for (const auto & i : txt)
  {
    if (i.type() == pugi::node_element && std::string("html") == i.name())
    {
      l = layoutXML_HTML(i, rules, shape);
    }
    else
    {
      throw XhtmlException_c("Top level tag must be the html tag (" + getNodePath(i) + ")");
    }
  }

  return l;
}

textLayout_c layoutXHTML(const std::string & txt, const textStyleSheet_c & rules, const shape_c & shape)
{
  pugi::xml_document doc;

  auto res = doc.load_buffer(txt.c_str(), txt.length(), pugi::parse_ws_pcdata | pugi::parse_escapes);

  if (res)
    return layoutXML(doc, rules, shape);
  else
  {
    throw XhtmlException_c(std::string("Error Parsing XHTML [") + doc.child("node").attribute("attr").value() + "]\n" +
            "Error description: " + res.description() + "\n" +
            "Error offset: " + boost::lexical_cast<std::string>(res.offset) + "  " +
            txt.substr(std::max<int>(res.offset-20, 0), 20) + "[here]" + txt.substr(res.offset, 20));
  }
}

}
