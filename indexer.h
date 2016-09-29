/* XDGSearch is a XAPIAN based file indexer and search tool.

    Copyright (C) 2016  Franco Martelli

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INDEXER_H
#define INDEXER_H

#include <memory>
#include <sstream>
#include <xapian.h>
#include <QTemporaryDir>
#include "configuration.h"

namespace XDGSearch {
class indexer_base;
class indexer;
void forEachHelper(const XDGSearch::helperType&, const XDGSearch::poolType&, Xapian::WritableDatabase*);
}

class XDGSearch::indexer_base final {
friend class indexer;
    class queryResult;
    explicit indexer_base(const Pool&);
    void populateDB() const;
    void seek(const std::string&);
    const Xapian::MSet enqueryDB(const std::string&) const;
    std::unique_ptr<XDGSearch::Configuration> conf;
    XDGSearch::poolType currentPoolSettings;
    std::string   xdgKey
                , htmlResult;
};

class XDGSearch::indexer_base::queryResult final   {
public:
    queryResult(const std::string&, const Xapian::MSet&);
    std::string getResult() const   { return htmlResult; }
private:
    std::string composeResult(const Xapian::MSet&);
    std::string   htmlResult
                , soughtTerms;
};

class XDGSearch::indexer final
{
public:
    indexer(const XDGSearch::Pool&);
    void populateDB() const         { return d ->populateDB(); }
    void seek(const std::string& s) { return d ->seek(s); }
    std::string getResult() const   { return d ->htmlResult; }
private:
    std::unique_ptr<XDGSearch::indexer_base> d;
};

inline
XDGSearch::indexer_base::queryResult::queryResult(const std::string& s, const Xapian::MSet& m) : soughtTerms(s)   {
    htmlResult = composeResult(m);
}
inline
XDGSearch::indexer::indexer(const XDGSearch::Pool& p = XDGSearch::Pool::END) :
    d(std::unique_ptr<XDGSearch::indexer_base>(new indexer_base(p)))
{

}
inline
void XDGSearch::indexer_base::seek(const std::string& s)
{
    XDGSearch::indexer_base::queryResult qr(s, enqueryDB(s));
    htmlResult = qr.getResult();
}


#endif // INDEXER_H
