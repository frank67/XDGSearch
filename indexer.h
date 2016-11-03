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
#include <xapian.h>
#include <QTemporaryDir>
#include "configuration.h"

namespace XDGSearch {
class IndexerBase;          // "Cheshire Cat" implemention class for Indexer class
class Indexer;              // Interface class for indexing/quering  operation
void forEachHelper( const XDGSearch::helperType&
                  , const XDGSearch::poolType&
                  , Xapian::WritableDatabase* );    // threaded function to populate each pool's database
}

class XDGSearch::IndexerBase final {
friend class Indexer;
    class queryResult;      // nested class to provide answer for sought terms
    explicit IndexerBase(const Pool&);
    void populateDB() const;    // build database for the current pool
    void seek(const std::string&);  // build a queryresult object and write result to htmlResult string
    const Xapian::MSet enqueryDB(const std::string&) const; // find a string in the current pool's database
    std::unique_ptr<XDGSearch::Configuration> conf;
    XDGSearch::poolType currentPoolSettings;
    std::string xdgKey, htmlResult;
};

class XDGSearch::IndexerBase::queryResult final   {
public:
    queryResult(const std::string&, const Xapian::MSet&);   // ctor that handle the query answers
    std::string getResult() const   { return htmlResult; }  // return an html formatted text suitable to an html viewer widget
private:
    std::string composeResult(const Xapian::MSet&);     // translate the query answer to an html formatted string
    std::string  htmlResult, soughtTerms;
};

class XDGSearch::Indexer final  {
public:
    Indexer(const XDGSearch::Pool&);
    void populateDB() const         { return d ->populateDB(); }
    void seek(const std::string& s) { d ->seek(s); }
    std::string getResult() const   { return d ->htmlResult; }
private:
    std::unique_ptr<XDGSearch::IndexerBase> d;
};

inline
XDGSearch::IndexerBase::queryResult::queryResult(const std::string& s, const Xapian::MSet& m) :
    soughtTerms(s)  { htmlResult = composeResult(m); }

inline
XDGSearch::Indexer::Indexer(const XDGSearch::Pool& p = XDGSearch::Pool::END) :
    d(std::unique_ptr<XDGSearch::IndexerBase>(new IndexerBase(p)))      { }

inline
void XDGSearch::IndexerBase::seek(const std::string& s)
{
    XDGSearch::IndexerBase::queryResult qr(s, enqueryDB(s));
    htmlResult = qr.getResult();
}

#endif // INDEXER_H
