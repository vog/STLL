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


static const std::vector<std::string> NamedSym_names  {
   "quot;",     "amp;",      "apos;",     "lt;",       "gt;",       "nbsp;",     "iexcl;",    "cent;",     "pound;",   "curren;",
   "yen;",      "brvbar;",   "sect;",     "uml;",      "copy;",     "ordf;",     "laquo;",    "not;",      "shy;",     "reg;",
   "macr;",     "deg;",      "plusmn;",   "sup2;",     "sup3;",     "acute;",    "micro;",    "para;",     "middot;",  "cedil;",
   "sup1;",     "ordm;",     "raquo;",    "frac14;",   "frac12;",   "frac34;",   "iquest;",   "Agrave;",   "Aacute;",  "Acirc;",
   "Atilde;",   "Auml;",     "Aring;",    "AElig;",    "Ccedil;",   "Egrave;",   "Eacute;",   "Ecirc;",    "Euml;",    "Igrave;",
   "Iacute;",   "Icirc;",    "Iuml;",     "ETH;",      "Ntilde;",   "Ograve;",   "Oacute;",   "Ocirc;",    "Otilde;",  "Ouml;",
   "times;",    "Oslash;",   "Ugrave;",   "Uacute;",   "Ucirc;",    "Uuml;",     "Yacute;",   "THORN;",    "szlig;",   "agrave;",
   "aacute;",   "acirc;",    "atilde;",   "auml;",     "aring;",    "aelig;",    "ccedil;",   "egrave;",   "eacute;",  "ecirc;",
   "euml;",     "igrave;",   "iacute;",   "icirc;",    "iuml;",     "eth;",      "ntilde;",   "ograve;",   "oacute;",  "ocirc;",
   "otilde;",   "ouml;",     "divide;",   "oslash;",   "ugrave;",   "uacute;",   "ucirc;",    "uuml;",     "yacute;",  "thorn;",
   "yuml;",     "OElig;",    "oelig;",    "Scaron;",   "scaron;",   "Yuml;",     "fnof;",     "circ;",     "tilde;",   "Alpha;",
   "Beta;",     "Gamma;",    "Delta;",    "Epsilon;",  "Zeta;",     "Eta;",      "Theta;",    "Iota;",     "Kappa;",   "Lambda;",
   "Mu;",       "Nu;",       "Xi;",       "Omicron;",  "Pi;",       "Rho;",      "Sigma;",    "Tau;",      "Upsilon;", "Phi;",
   "Chi;",      "Psi;",      "Omega;",    "alpha;",    "beta;",     "gamma;",    "delta;",    "epsilon;",  "zeta;",    "eta;",
   "theta;",    "iota;",     "kappa;",    "lambda;",   "mu;",       "nu;",       "xi;",       "omicron;",  "pi;",      "rho;",
   "sigmaf;",   "sigma;",    "tau;",      "upsilon;",  "phi;",      "chi;",      "psi;",      "omega;",    "thetasym;","upsih;",
   "piv;",      "ensp;",     "emsp;",     "thinsp;",   "zwnj;",     "zwj;",      "lrm;",      "rlm;",      "ndash;",   "mdash;",
   "lsquo;",    "rsquo;",    "sbquo;",    "ldquo;",    "rdquo;",    "bdquo;",    "dagger;",   "Dagger;",   "bull;",    "hellip;",
   "permil;",   "prime;",    "Prime;",    "lsaquo;",   "rsaquo;",   "oline;",    "frasl;",    "euro;",     "image;",   "weierp;",
   "real;",     "trade;",    "alefsym;",  "larr;",     "uarr;",     "rarr;",     "darr;",     "harr;",     "crarr;",   "lArr;",
   "uArr;",     "rArr;",     "dArr;",     "hArr;",     "forall;",   "part;",     "exist;",    "empty;",    "nabla;",   "isin;",
   "notin;",    "ni;",       "prod;",     "sum;",      "minus;",    "lowast;",   "radic;",    "prop;",     "infin;",   "ang;",
   "and;",      "or;",       "cap;",      "cup;",      "int;",      "there4;",   "sim;",      "cong;",     "asymp;",   "ne;",
   "equiv;",    "le;",       "ge;",       "sub;",      "sup;",      "nsub;",     "sube;",     "supe;",     "oplus;",   "otimes;",
   "perp;",     "sdot;",     "vellip;",   "lceil;",    "rceil;",    "lfloor;",   "rfloor;",   "lang;",     "rang;",    "loz;",
   "spades;",   "clubs;",    "hearts;",   "diams;",
};

static const std::vector<const char *> NamedSym_values {
   "\u0022",    "\u0026",    "\u0027",    "\u003C",    "\u003E",    "\u00A0",    "\u00A1",    "\u00A2",    "\u00A3",    "\u00A4",
   "\u00A5",    "\u00A6",    "\u00A7",    "\u00A8",    "\u00A9",    "\u00AA",    "\u00AB",    "\u00AC",    "\u00AD",    "\u00AE",
   "\u00AF",    "\u00B0",    "\u00B1",    "\u00B2",    "\u00B3",    "\u00B4",    "\u00B5",    "\u00B6",    "\u00B7",    "\u00B8",
   "\u00B9",    "\u00BA",    "\u00BB",    "\u00BC",    "\u00BD",    "\u00BE",    "\u00BF",    "\u00C0",    "\u00C1",    "\u00C2",
   "\u00C3",    "\u00C4",    "\u00C5",    "\u00C6",    "\u00C7",    "\u00C8",    "\u00C9",    "\u00CA",    "\u00CB",    "\u00CC",
   "\u00CD",    "\u00CE",    "\u00CF",    "\u00D0",    "\u00D1",    "\u00D2",    "\u00D3",    "\u00D4",    "\u00D5",    "\u00D6",
   "\u00D7",    "\u00D8",    "\u00D9",    "\u00DA",    "\u00DB",    "\u00DC",    "\u00DD",    "\u00DE",    "\u00DF",    "\u00E0",
   "\u00E1",    "\u00E2",    "\u00E3",    "\u00E4",    "\u00E5",    "\u00E6",    "\u00E7",    "\u00E8",    "\u00E9",    "\u00EA",
   "\u00EB",    "\u00EC",    "\u00ED",    "\u00EE",    "\u00EF",    "\u00F0",    "\u00F1",    "\u00F2",    "\u00F3",    "\u00F4",
   "\u00F5",    "\u00F6",    "\u00F7",    "\u00F8",    "\u00F9",    "\u00FA",    "\u00FB",    "\u00FC",    "\u00FD",    "\u00FE",
   "\u00FF",    "\u0152",    "\u0153",    "\u0160",    "\u0161",    "\u0178",    "\u0192",    "\u02C6",    "\u02DC",    "\u0391",
   "\u0392",    "\u0393",    "\u0394",    "\u0395",    "\u0396",    "\u0397",    "\u0398",    "\u0399",    "\u039A",    "\u039B",
   "\u039C",    "\u039D",    "\u039E",    "\u039F",    "\u03A0",    "\u03A1",    "\u03A3",    "\u03A4",    "\u03A5",    "\u03A6",
   "\u03A7",    "\u03A8",    "\u03A9",    "\u03B1",    "\u03B2",    "\u03B3",    "\u03B4",    "\u03B5",    "\u03B6",    "\u03B7",
   "\u03B8",    "\u03B9",    "\u03BA",    "\u03BB",    "\u03BC",    "\u03BD",    "\u03BE",    "\u03BF",    "\u03C0",    "\u03C1",
   "\u03C2",    "\u03C3",    "\u03C4",    "\u03C5",    "\u03C6",    "\u03C7",    "\u03C8",    "\u03C9",    "\u03D1",    "\u03D2",
   "\u03D6",    "\u2002",    "\u2003",    "\u2009",    "\u200C",    "\u200D",    "\u200E",    "\u200F",    "\u2013",    "\u2014",
   "\u2018",    "\u2019",    "\u201A",    "\u201C",    "\u201D",    "\u201E",    "\u2020",    "\u2021",    "\u2022",    "\u2026",
   "\u2030",    "\u2032",    "\u2033",    "\u2039",    "\u203A",    "\u203E",    "\u2044",    "\u20AC",    "\u2111",    "\u2118",
   "\u211C",    "\u2122",    "\u2135",    "\u2190",    "\u2191",    "\u2192",    "\u2193",    "\u2194",    "\u21B5",    "\u21D0",
   "\u21D1",    "\u21D2",    "\u21D3",    "\u21D4",    "\u2200",    "\u2202",    "\u2203",    "\u2205",    "\u2207",    "\u2208",
   "\u2209",    "\u220B",    "\u220F",    "\u2211",    "\u2212",    "\u2217",    "\u221A",    "\u221D",    "\u221E",    "\u2220",
   "\u2227",    "\u2228",    "\u2229",    "\u222A",    "\u222B",    "\u2234",    "\u223C",    "\u2245",    "\u2248",    "\u2260",
   "\u2261",    "\u2264",    "\u2265",    "\u2282",    "\u2283",    "\u2284",    "\u2286",    "\u2287",    "\u2295",    "\u2297",
   "\u22A5",    "\u22C5",    "\u22EE",    "\u2308",    "\u2309",    "\u230A",    "\u230B",    "\u2329",    "\u232A",    "\u25CA",
   "\u2660",    "\u2663",    "\u2665",    "\u2666",
};


class indentShape_c : public Shape_c
{
  private:
    const Shape_c & outside;
    int32_t ind_left, ind_right;

  public:
    indentShape_c(const Shape_c & s, int32_t li, int32_t ri) : outside(s), ind_left(li), ind_right(ri) { }

    virtual int32_t getLeft(int32_t top, int32_t bottom) const { return outside.getLeft(top, bottom)+ind_left; }
    virtual int32_t getRight(int32_t top, int32_t bottom) const { return outside.getRight(top, bottom)-ind_right; }
    virtual int32_t getLeft2(int32_t top, int32_t bottom) const { return outside.getLeft2(top, bottom)+ind_left; }
    virtual int32_t getRight2(int32_t top, int32_t bottom) const { return outside.getRight2(top, bottom)-ind_right; }
};

class stripLeftShape_c : public Shape_c
{
  private:
    const Shape_c & outside;
    int32_t ind_left, ind_right;

  public:
    stripLeftShape_c(const Shape_c & s, int32_t li, int32_t ri) : outside(s), ind_left(li), ind_right(ri) { }

    virtual int32_t getLeft(int32_t top, int32_t bottom) const { return outside.getLeft(top, bottom)+ind_left; }
    virtual int32_t getRight(int32_t top, int32_t bottom) const { return outside.getLeft(top, bottom)+ind_right; }
    virtual int32_t getLeft2(int32_t top, int32_t bottom) const { return outside.getLeft2(top, bottom)+ind_left; }
    virtual int32_t getRight2(int32_t top, int32_t bottom) const { return outside.getLeft2(top, bottom)+ind_right; }
};

class stripRightShape_c : public Shape_c
{
  private:
    const Shape_c & outside;
    int32_t ind_left, ind_right;

  public:
    stripRightShape_c(const Shape_c & s, int32_t li, int32_t ri) : outside(s), ind_left(li), ind_right(ri) { }

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

static uint8_t dec2num(char c)
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
    default:
      throw XhtmlException_c("Wrong format for a decimal-number");
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
    virtual double operator()(void) const { return 0; };
};


/** \brief evaluate size
 *  \param sz the size string from the CSS
 *  \return the resulting size in pixel
 */
template <class T = szFunctor>
static double evalSize(const std::string & sz, T f = szFunctor())
{
  // right now we accept only pixel sizes
  size_t l = sz.length();

  if (sz[l-2] == 'p' && sz[l-1] == 'x')
  {
    return 64*atof(sz.c_str());
  }
  else if (sz[l-1] == '%')
  {
    return f() * atof(sz.c_str()) / 100;
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

    double operator()(void) const
    {
      if (!node)
        throw XhtmlException_c("no parent node to base a percent value on");

      parentFunctor f(tag, node.parent(), rules);
      return evalSize(rules.getValue(node, tag), f);
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

  for (size_t j = 0; j < in.size(); j++)
  {
    auto a = in[j];

    if (a == '\n' || a == '\r')
      a = ' ';

    if (a == '&')
    {
      bool found = false;

      // check for a named character
      for (size_t i = 0; i < NamedSym_names.size(); i++)
        if (in.compare(j+1, NamedSym_names[i].size(), NamedSym_names[i]) == 0)
        {
          out += NamedSym_values[i];
          j += NamedSym_names[i].size();
          found = true;
          break;
        }

      // check, if there is a universal number type character
      if (!found)
      {
        if (j+1 < in.size() && in[j+1] == '#')
        {
          int32_t num = 0;

          if (j+2 < in.size() && in[j+2] == 'x')
          {
            int k = j+3;
            while (k < in.size())
            {
              if (in[k] == ';')
              {
                found = true;
                j = k;
                break;
              }
              num = num * 16 + hex2num(in[k]);
              k++;
            }
          }
          else
          {
            int k = j+2;
            while (k < in.size())
            {
              if (in[k] == ';')
              {
                found = true;
                j = k;
                break;
              }
              num = num * 10 + dec2num(in[k]);
              k++;
            }
          }

          if (found)
            out += U32ToUTF8(num);
        }
      }

      if (!found)
        out += a;
    }
    else if (a != ' ' || prev != ' ')
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
  double fontSize = evalSize(rules.getValue(xml, "font-size"), fkt);

  auto fam = rules.findFamily(fontFamily);

  if (fam)
  {
    auto f = fam->getFont(fontSize, fontStyle, fontVariant, fontWeight);

    if (f) return f;
  }

  throw XhtmlException_c(std::string("Requested font not found (family:'") + fontFamily +
                                     "', style: '" + fontStyle +
                                     "', variant: '" + fontVariant +
                                     "', weight: '" + fontWeight + ") required here: " + getNodePath(xml));

  return 0;
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

typedef TextLayout_c (*ParseFunction)(pugi::xml_node & xml, const textStyleSheet_c & rules,
                                      const Shape_c & shape, int32_t ystart);


// handles padding, margin and border, all in one, it takes the text returned from the
// ParseFunction and boxes it
static TextLayout_c boxIt(const pugi::xml_node & xml, pugi::xml_node & xml2, const textStyleSheet_c & rules,
                          const Shape_c & shape, int32_t ystart, ParseFunction fkt,
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

  int space = minHeight - (l2.getHeight()+padding_bottom+borderwidth_bottom+margin_bottom);
  l2.setHeight(std::max(minHeight, l2.getHeight()+padding_bottom+borderwidth_bottom+margin_bottom));

  if (space > 0)
  {
    // TODO baseline is missing
         if (rules.getValue(xml, "vertical-align") == "bottom") l2.shift(0, space);
    else if (rules.getValue(xml, "vertical-align") == "middle") l2.shift(0, space/2);
  }

  if (borderwidth_top)
  {
    std::string color = rules.getValue(xml, "border-color");
    if (rules.getValue(xml, "border-top-color") != "") color = rules.getValue(xml, "border-top-color");
    if (color == "")                                   color = rules.getValue(xml, "color");
    auto cc = evalColor(color);

    if (cc.a() != 0)
    {
      int32_t cx = l2.getLeft()-padding_left-borderwidth_left;
      int32_t cy = ystart+margin_top;
      int32_t cw = l2.getRight()-l2.getLeft()+padding_left+padding_right+borderwidth_left+borderwidth_right;
      int32_t ch = borderwidth_top;
      l2.addCommandStart(CommandData_c(cx, cy, cw, ch, cc));
    }
  }

  if (borderwidth_bottom)
  {
    std::string color = rules.getValue(xml, "border-color");
    if (rules.getValue(xml, "border-bottom-color") != "") color = rules.getValue(xml, "border-bottom-color");
    if (color == "")                                      color = rules.getValue(xml, "color");
    auto cc = evalColor(color);

    if (cc.a() != 0)
    {
      int32_t cx = l2.getLeft()-padding_left-borderwidth_left;
      int32_t cy = l2.getHeight()-borderwidth_bottom-margin_bottom;
      int32_t cw = l2.getRight()-l2.getLeft()+padding_left+padding_right+borderwidth_left+borderwidth_right;
      int32_t ch = borderwidth_bottom;
      l2.addCommandStart(CommandData_c(cx, cy, cw, ch, cc));
    }
  }

  if (borderwidth_right)
  {
    std::string color = rules.getValue(xml, "border-color");
    if (rules.getValue(xml, "border-right-color") != "") color = rules.getValue(xml, "border-right-color");
    if (color == "")                                     color = rules.getValue(xml, "color");
    auto cc = evalColor(color);

    if (cc.a() != 0)
    {
      int32_t cx = l2.getRight()+padding_right;
      int32_t cy = ystart+margin_top;
      int32_t cw = borderwidth_right;
      int32_t ch = l2.getHeight()-ystart-margin_bottom-margin_top;
      l2.addCommandStart(CommandData_c(cx, cy, cw, ch, cc));
    }
  }

  if (borderwidth_left)
  {
    std::string color = rules.getValue(xml, "border-color");
    if (rules.getValue(xml, "border-left-color") != "") color = rules.getValue(xml, "border-left-color");
    if (color == "")                                    color = rules.getValue(xml, "color");
    auto cc = evalColor(color);

    if (cc.a() != 0)
    {
      int32_t cx = l2.getLeft()-padding_left-borderwidth_left;
      int32_t cy = ystart+margin_top;
      int32_t cw = borderwidth_left;
      int32_t ch = l2.getHeight()-ystart-margin_bottom-margin_top;
      l2.addCommandStart(CommandData_c(cx, cy, cw, ch, cc));
    }
  }

  auto cc = evalColor(rules.getValue(xml, "background-color"));

  if (cc.a() != 0)
  {
    int32_t cx = shape.getLeft(ystart+margin_top, ystart+margin_top)+borderwidth_left+margin_left;
    int32_t cy = ystart+borderwidth_top+margin_top;
    int32_t cw = shape.getRight(ystart+margin_top, ystart+margin_top)-
                 shape.getLeft(ystart+margin_top, ystart+margin_top)-borderwidth_right-borderwidth_left-margin_right-margin_left;
    int32_t ch = l2.getHeight()-ystart-borderwidth_bottom-borderwidth_top-margin_bottom-margin_top;
    l2.addCommandStart(CommandData_c(cx, cy, cw, ch, cc));
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

static TextLayout_c layoutXML_IMG(pugi::xml_node & xml, const textStyleSheet_c & rules,
                                  const Shape_c & shape, int32_t ystart)
{
  TextLayout_c l;

  int32_t cx = shape.getLeft(ystart, ystart);
  int32_t cy = ystart;
  int32_t cw = evalSize(xml.attribute("width").value());
  int32_t ch = evalSize(xml.attribute("height").value());
  auto ci = xml.attribute("src").value();

  l.addCommand(CommandData_c(ci, cx, cy, cw, ch));
  l.setHeight(ystart+ch);
  l.setLeft(cx);
  l.setRight(cx+cw);

  return l;
}

// this function is different from all the other layout functions as it
// will take the first XML-node to layout and work along with the siblings
// instead of looking at the children
// this function will also return a new node where it stopped working
pugi::xml_node layoutXML_text(pugi::xml_node xml, const textStyleSheet_c & rules,
                              layoutProperties & prop, std::u32string & txt,
                              AttributeIndex_c & attr, int32_t baseline = 0,
                              const std::string & link = "", bool exitOnError = false)
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

      if (!link.empty())
      {
        prop.links.push_back(link);
        a.link = prop.links.size();
      }

      attr.set(s, txt.length()-1, a);
    }
    else if (   (xml.type() == pugi::node_element)
             && (   (std::string("i") == xml.name())
                 || (std::string("span") == xml.name())
                 || (std::string("b") == xml.name())
                 || (std::string("code") == xml.name())
                 || (std::string("em") == xml.name())
                 || (std::string("q") == xml.name())
                 || (std::string("small") == xml.name())
                 || (std::string("strong") == xml.name())
                 || (std::string("a") == xml.name())
                )
            )
    {
      if (rules.getValue(xml, "direction") == "rtl")
      {
        txt += U"\U0000202B";
      }
      else
      {
        txt += U"\U0000202A";
      }

      if (std::string("a") == xml.name())
      {
        layoutXML_text(xml.first_child(), rules, prop, txt, attr, baseline, xml.attribute("href").value());
      }
      else
      {
        layoutXML_text(xml.first_child(), rules, prop, txt, attr, baseline, link);
      }
      txt += U"\U0000202C";
    }
    else if (   (xml.type() == pugi::node_element)
             && (std::string("sub") == xml.name())
            )
    {
      auto font = getFontForNode(xml, rules);

      layoutXML_text(xml.first_child(), rules, prop, txt, attr, baseline-font->getAscender()/2, link);
    }
    else if (   (xml.type() == pugi::node_element)
             && (std::string("sup") == xml.name())
            )
    {
      auto font = getFontForNode(xml.parent(), rules);

      layoutXML_text(xml.first_child(), rules, prop, txt, attr, baseline+font->getAscender()/2, link);
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
      a.inlay = std::make_shared<TextLayout_c>(boxIt(xml, xml, rules, RectangleShape_c(10000), 0,
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

      if (!link.empty())
      {
        prop.links.push_back(link);
        a.link = prop.links.size();
      }

      txt += U'\u00A0';
      attr.set(txt.length()-1, a);
    }
    else
    {
      if (exitOnError)
        break;
      else
        throw XhtmlException_c("Found non phrasing element in phrasing context (" + getNodePath(xml) + ")" );
    }

    xml = xml.next_sibling();
  }
  return xml;
}

// this function is different from all other layout functions usable in the boxIt
// function, as it will change the xml node and return a new one
static TextLayout_c layoutXML_Phrasing(pugi::xml_node & xml, const textStyleSheet_c & rules, const Shape_c & shape, int32_t ystart)
{
  std::u32string txt;
  AttributeIndex_c attr;
  layoutProperties lprop;

  auto xml2 = layoutXML_text(xml, rules, lprop, txt, attr, 0, "", true);

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
  lprop.underlineFont = getFontForNode(xml.parent(), rules);

  xml = xml2;

  return layoutParagraph(txt, attr, shape, lprop, ystart);
}

static TextLayout_c layoutXML_Flow(pugi::xml_node & txt, const textStyleSheet_c & rules, const Shape_c & shape, int32_t ystart);

static TextLayout_c layoutXML_UL(pugi::xml_node & xml, const textStyleSheet_c & rules, const Shape_c & shape, int32_t ystart)
{
  TextLayout_c l;
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

      auto font = getFontForNode(i, rules);
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

      std::unique_ptr<Shape_c> bulletshape;

      if (direction == "ltr")
      {
        bulletshape.reset(new stripLeftShape_c(shape, padding, padding+listIndent));
      }
      else
      {
        bulletshape.reset(new stripRightShape_c(shape, padding+listIndent, padding));
      }

      indentShape_c textshape(shape, direction == "ltr" ? listIndent : 0, direction == "ltr" ? 0: listIndent);

      TextLayout_c bullet = layoutParagraph(U"\u2022", AttributeIndex_c(a), *bulletshape.get(), prop, y+padding);
      TextLayout_c text = boxIt(i, i, rules, textshape, y, layoutXML_Flow, i.previous_sibling(), pugi::xml_node());

      // append the bullet first and then the text, adjusting the bullet so that its baseline
      // is at the same vertical position as the first baseline in the text
      l.append(bullet, 0, text.getFirstBaseline() - bullet.getFirstBaseline());
      l.append(text);

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

  TextLayout_c l;

} tableCell;

static void layoutXML_TR(pugi::xml_node & xml, uint32_t row, const textStyleSheet_c & /* rules */,
                         std::vector<tableCell> & cells, vector2d<pugi::xml_node> & cellarray,
                         size_t columns)
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

      if (col > columns)
      {
        throw XhtmlException_c("You must not use more columns that specified in the colgroup tag (" + getNodePath(i) + ")");
      }
    }
    else
    {
      throw XhtmlException_c("Only 'th' or 'td' tags allowed within 'ul' tag (" + getNodePath(i) + ")");
    }
  }
}

static TextLayout_c layoutXML_TABLE(pugi::xml_node & xml, const textStyleSheet_c & rules, const Shape_c & shape, int32_t ystart)
{
  std::vector<tableCell> cells;
  std::vector<uint32_t> widths;
  std::vector<uint32_t> colStart;
  vector2d<pugi::xml_node> cellarray;
  uint32_t row = 0;
  bool col = false;
  bool rtl = (rules.getValue(xml, "direction") == "rtl");
  int left = rtl ? 1 : -1;

  std::string defaulttablewidth("100%");
  auto tablew = rules.getValue(xml, "width", defaulttablewidth);
  double table_width = evalSize(tablew, [&shape, ystart] (void)->double {
    return shape.getRight(ystart, ystart) - shape.getLeft(ystart, ystart);
  });

  // collect all cells of the table
  for (auto & i : xml)
  {
    if (   (i.type() == pugi::node_element)
        && (std::string("colgroup") == i.name())
       )
    {
      std::vector<double> relativeWidths;

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

          auto w = rules.getValue(j, "width");

          if (w.back() == '*')
          {
            double width = std::stod(w.substr(0, w.length()-1).c_str());

            while (span > 0)
            {
              widths.push_back(0);
              relativeWidths.push_back(width);
              span--;
            }
          }
          else
          {
            uint32_t width = evalSize(w, [table_width] (void) {
              return table_width;
            });

            while (span > 0)
            {
              widths.push_back(width);
              relativeWidths.push_back(0);
              span--;
            }
          }
        }
        else
        {
          throw XhtmlException_c("Only 'col' tags allowed within 'colgroup' tag (" + getNodePath(j) + ")");
        }
      }

      assert(relativeWidths.size() == widths.size());

      // handle relative widths
      // 1) calculate the remaining width available for the table
      uint32_t remainingWidth = 0;
      for (auto w : widths)
        remainingWidth += w;

      if (remainingWidth < table_width)
      {
        remainingWidth = table_width - remainingWidth;

        // 2) calculate the sum of all relative widths
        double relSum = 0.0;
        for (auto w : relativeWidths)
          relSum += w;

        // 3) distribute the space
        if (relSum > 0.01)
        {
          for (int i = 0; i < relativeWidths.size(); i++)
          {
            widths[i] += remainingWidth * relativeWidths[i]/relSum;
          }
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

      layoutXML_TR(i, row, rules, cells, cellarray, widths.size());
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
    c.l = boxIt(c.xml, c.xml, rules, RectangleShape_c(colStart[c.col+c.colspan]-colStart[c.col]),
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
  TextLayout_c l;
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
      c.l = boxIt(c.xml, c.xml, rules, RectangleShape_c(colStart[c.col+c.colspan]-colStart[c.col]),
                  0, layoutXML_Flow, cellarray.get(c.col+1, c.row), cellarray.get(c.col+(1+left)*c.colspan, c.row+1),
                  rules.getValue(xml, "border-collapse") == "collapse", rh);

    if (l.getData().empty())
      l.setFirstBaseline(c.l.getFirstBaseline()+ystart);

    if (rtl)
    {
      l.append(c.l, xindent+*colStart.rbegin()-colStart[1]+colStart[0]-colStart[c.col+c.colspan-1], ystart);
    }
    else
    {
      l.append(c.l, colStart[c.col]+xindent, ystart);
    }
  }

  l.setHeight(ystart+rowheights[row]);
  l.setLeft(xindent+colStart[0]);
  l.setRight(xindent+width);

  return l;
}

static TextLayout_c layoutXML_Flow(pugi::xml_node & txt, const textStyleSheet_c & rules, const Shape_c & shape, int32_t ystart)
{
  TextLayout_c l;
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
                &&(  (std::string("span") == i.name())
                   ||(std::string("b") == i.name())
                   ||(std::string("br") == i.name())
                   ||(std::string("code") == i.name())
                   ||(std::string("em") == i.name())
                   ||(std::string("q") == i.name())
                   ||(std::string("small") == i.name())
                   ||(std::string("strong") == i.name())
                   ||(std::string("sub") == i.name())
                   ||(std::string("sup") == i.name())
                   ||(std::string("img") == i.name())
                   ||(std::string("a") == i.name())
                  )
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

static TextLayout_c layoutXML_HTML(const pugi::xml_node & txt, const textStyleSheet_c & rules, const Shape_c & shape)
{
  TextLayout_c l;

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

TextLayout_c layoutXML(const pugi::xml_document & txt, const textStyleSheet_c & rules, const Shape_c & shape)
{
  TextLayout_c l;

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

TextLayout_c layoutXHTML(const std::string & txt, const textStyleSheet_c & rules, const Shape_c & shape)
{
  pugi::xml_document doc;

  auto res = doc.load_buffer(txt.c_str(), txt.length(), pugi::parse_ws_pcdata);

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
