/* XDGSearch is a XAPIAN based file indexer and search tool.

    Copyright (C) 2016,2017  Franco Martelli

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

#ifndef XDGSEARCH_INCLUDED_INDEXER_H
#define XDGSEARCH_INCLUDED_INDEXER_H

#include <memory>
#include <xapian.h>
#include <QTemporaryDir>
#include <QObject>
#include "configuration.h"

namespace XDGSearch {
class IndexerBase;          /// "Cheshire Cat" implemention class for Indexer class
class Indexer;              /// Interface class for indexing/quering  operation
}

class XDGSearch::IndexerBase final : public QObject {
    Q_OBJECT
friend class Indexer;
    class queryResult;      /// nested class to provide answer for sought terms
    IndexerBase(QObject*, const Pool&);
    void populateDB();      /// build database for the current pool
    void forEachHelper( const XDGSearch::helperType&
                      , const XDGSearch::poolType&
                      , Xapian::WritableDatabase* );    /// threaded function to populate each pool's database
    void seek(const std::string&);  /// build a queryresult object and write result to htmlResult string
    const Xapian::MSet enqueryDB(const std::string&) const; /// find a string in the current pool's database
    void setProgressBarValue();     /// determines values useful for the progress bar
    void resetProgressBarValue();   /// sets values for the progress bar to its initial values
    double p_value, p_valueStep;    /// p_value: the value passed to setvalue member, p_valueStep: progress bar increment step
    std::unique_ptr<XDGSearch::Configuration> const conf;
    XDGSearch::poolType currentPoolSettings;
    std::string xdgKey, htmlResult;
signals:
    void progressValue(int);
};

class XDGSearch::IndexerBase::queryResult final   {
public:
    queryResult(const std::string&, const Xapian::MSet&);   /// ctor that handle the query answers
    std::string getResult() const   { return htmlResult; }  /// return an html formatted text suitable to an html viewer widget
private:
    std::string composeResult(const Xapian::MSet&);     /// translate the query answer to an html formatted string
    std::string htmlResult, soughtTerms;
};

class XDGSearch::Indexer final : public QObject {
    Q_OBJECT
public:
    Indexer(QObject*, const XDGSearch::Pool&);
    ~Indexer();
    void populateDB()         { return d ->populateDB(); }
    void seek(const std::string& s) { d ->seek(s); }
    std::string getResult() const   { return d ->htmlResult; }
signals:
    void progressValue(int);
private:
    XDGSearch::IndexerBase* const d;
};

inline
XDGSearch::IndexerBase::queryResult::queryResult(const std::string& s, const Xapian::MSet& m) :
    soughtTerms(s)  { htmlResult = composeResult(m); }

inline
void XDGSearch::IndexerBase::seek(const std::string& s)
{
    const XDGSearch::IndexerBase::queryResult qr(s, enqueryDB(s));
    htmlResult = qr.getResult();
}

#endif /// XDGSEARCH_INCLUDED_INDEXER_H
