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

#include "indexer.h"

#include <QDirIterator>
#include <QStringList>
#include <forward_list>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <new>      // used for bad_alloc
#include <thread>
#include <mutex>
//#include <memory>



XDGSearch::indexer_base::indexer_base(const Pool& p) :
        conf(std::unique_ptr<XDGSearch::Configuration>(new XDGSearch::Configuration(p)))
{
    currentPoolSettings = conf ->enqueryPool();
}

void XDGSearch::indexer_base::populateDB() const
{
    QTemporaryDir tempDir;
    while(!tempDir.isValid())
        ;
    std::istringstream iss(std::get<2>(currentPoolSettings));
    std::forward_list<XDGSearch::helperType> poolHelpers;
    std::forward_list<std::unique_ptr<std::thread>> helpersThread;

    const std::string  tmpDirName  =  tempDir.path().toStdString() + "/"
                     , DBName  =  std::get<1>(currentPoolSettings)
                     , tmpDBName  =  tmpDirName + DBName;
try {
    Xapian::WritableDatabase* tmpDB(new Xapian::WritableDatabase(  tmpDBName
                                                                 , Xapian::DB_CREATE_OR_OVERWRITE
                                                                 , Xapian::DB_BACKEND_CHERT));
    for(std::string h; std::getline(iss, h, ','); )
        poolHelpers.push_front(conf ->enqueryHelper(h));

    for(const auto& h : poolHelpers)    {   // outer loop, for each pool's helper
        if(std::get<0>(h).empty())
            continue;
        helpersThread.push_front(std::unique_ptr<std::thread>(new std::thread(  forEachHelper
                                                                              , h
                                                                              , currentPoolSettings
                                                                              , tmpDB )));
    }
    for(auto& t : helpersThread)
        t ->join();

    tmpDB ->compact(DBName);
    tmpDB ->close();
    delete tmpDB;
}
    catch(const Xapian::Error& e)  {
        std::cerr << e.get_description() << std::endl;
        exit(EXIT_FAILURE);
    }
    catch(const std::runtime_error& e)  {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    catch(const std::bad_alloc& e)  {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void XDGSearch::forEachHelper(  const XDGSearch::helperType& h
                              , const XDGSearch::poolType& currentPoolSettings
                              , Xapian::WritableDatabase* tmpDB)
{
    static std::unique_ptr<std::mutex> mtx1(new std::mutex);
    const std::string stopWordsFile = "stopwords/" + std::get<5>(currentPoolSettings);

    Xapian::WritableDatabase inRamDB(Xapian::InMemory::open());
    Xapian::Stem stemmer(std::get<4>(currentPoolSettings));
    Xapian::TermGenerator indexer;
    Xapian::SimpleStopper sstopper;
    std::fstream ifs(stopWordsFile);

    indexer.set_stemmer(stemmer);
    if(ifs) {
        for(std::string s; std::getline(ifs, s); /* null */)
            sstopper.add(s);
        indexer.set_stopper(&sstopper);
    }

    std::istringstream iss(std::get<1>(h));
    QStringList filesExtensionFilter;

    for(std::string e; std::getline(iss, e, ','); ) {
        e = "*." + e;
        filesExtensionFilter  << QString::fromStdString(e);
    }

    QDirIterator dirIt(  QString::fromStdString(std::get<3>(currentPoolSettings))
                       , filesExtensionFilter
                       , QDir::Files
                       , QDirIterator::Subdirectories | QDirIterator::FollowSymlinks);

    while(dirIt.hasNext())  {   // first inner loop, for each file into pool's directory
        const std::string  fileFullPathName = dirIt.next().toStdString()
                         , cmd = std::get<2>(h)
                               + " \""
                               + fileFullPathName
                               + "\"";
        std::string cmdStdOut;

        FILE* pipe = popen(cmd.c_str(), "r");
        try {
        if (!pipe)
            throw std::runtime_error("Helper: command line not valid!");
        for(char buffer[512]; !feof(pipe); )
            if (fgets(buffer, 512, pipe) != NULL)
                cmdStdOut += buffer;
        } catch (...) {
            pclose(pipe);
            throw;
        }
        pclose(pipe);

        unsigned int  linescounter = 0;
        const unsigned int granularity = std::get<3>(h)
                         , maxLinesCounted = granularity;
        std::istringstream(cmdStdOut).swap(iss);

        for(std::string line, para; !iss.eof(); ) { // second inner loop, for each line of helper's result file
            getline(iss, line);
            if(!iss.eof() && line.empty())
                continue;
            if(!granularity)    {
                if (iss.eof() || linescounter == 15) {

                    if(iss.eof())   {
                        para += ' ';
                        para += line;
                    }

                    Xapian::Document doc;
                    doc.set_data(para);
                    // see: https://trac.xapian.org/wiki/FAQ/UniqueIds
                    doc.add_term("P" + fileFullPathName);

                    indexer.set_document(doc);
                    indexer.index_text(para);

                    // Add the document to the database.
                    inRamDB.add_document(doc);

                    linescounter = 0;
                    break;

                } else {
                    if(!para.empty())
                        para += ' ';

                    para += line;
                    ++linescounter;
                }
            } else {
                if (iss.eof() || linescounter == maxLinesCounted) {

                    if(iss.eof())   {
                        para += ' ';
                        para += line;
                    }
                    Xapian::Document doc;
                    doc.set_data(para);
                    // see: https://trac.xapian.org/wiki/FAQ/UniqueIds
                    doc.add_term("P" + fileFullPathName);

                    indexer.set_document(doc);
                    indexer.index_text(para);

                    // Add the document to the database.
                    inRamDB.add_document(doc);

                    para.clear();
                    linescounter = 0;

                } else {
                    if(!para.empty())
                        para += ' ';

                    para += line;
                    ++linescounter;
                }
            }
        }
    }
    inRamDB.commit();

    mtx1 ->lock();

    for(  Xapian::PostingIterator it = inRamDB.postlist_begin(std::string())
          ; it != inRamDB.postlist_end(std::string())
          ; ++it )
        tmpDB ->add_document(inRamDB.get_document(*it));

    tmpDB ->commit();
    inRamDB.close();

    mtx1 ->unlock();
}

const Xapian::MSet XDGSearch::indexer_base::enqueryDB(const std::string& query_string) const   {
try {
    // Open the database for searching.
    Xapian::Database db(std::get<1>(currentPoolSettings));

    // Start an enquire session.
    Xapian::Enquire enquire(db);

    // Parse the query string to produce a Xapian::Query object.
    Xapian::QueryParser qp;
    Xapian::Stem stemmer(std::get<4>(currentPoolSettings));
    qp.set_stemmer(stemmer);
    qp.set_database(db);
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    Xapian::Query query = qp.parse_query(query_string);

    // Find the top 10 results for the query.
    enquire.set_query(query);
    Xapian::MSet matches = enquire.get_mset(0, 10);

    return matches;
    }
catch (const Xapian::Error &e) {
    std::cerr << e.get_description() << std::endl;
    exit(EXIT_FAILURE);
    }
}

std::string XDGSearch::indexer_base::queryResult::composeResult(const Xapian::MSet& matches)    {
try {
        std::ostringstream htmlCompose;
        htmlCompose << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                    << "<html><head><meta name=\"qrichtext\" content=\"1\" /><title>Result</title><style type=\"text/css\">\n"
                    << "p, li { white-space: pre-wrap; }\n"
                    << "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n";

        if(!matches.size())
            htmlCompose  << "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Not found</p></body></html>";
        else    {
            for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i) {
                auto term_iterator = i.get_document().termlist_begin();
                // advance until find the term added to the document with add_term (the path of the file)
                while(*(*term_iterator).cbegin() != 'P')
                    ++term_iterator;

                std::istringstream iss(*term_iterator);
                std::string   linkName
                            , linkPath;
                while(std::getline(iss, linkName, '/'))
                    if(linkName == "P")
                        linkPath = "file://";
                    else
                        linkPath = linkPath + "/" + linkName;

                QString data = QString::fromStdString(i.get_document().get_data());
                std::istringstream(soughtTerms).swap(iss);

                for(  std::string term
                    ; std::getline(iss, term)
                    ; /* null */)
                    for(  int pos = data.indexOf(QString::fromStdString(term), 0, Qt::CaseInsensitive)
                        ; pos != -1
                        ; /* null */)   {
                        data.insert(pos, "<span style=\" font-weight:600;\">");
                        pos = data.indexOf(QString::fromStdString(term), pos, Qt::CaseInsensitive);
                        pos += term.size();
                        data.insert(pos, "</span>");
                        ++pos;
                        pos = data.indexOf( QString::fromStdString(term)
                                          , pos
                                          , Qt::CaseInsensitive);
                    }

                htmlCompose << "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href="
                            << "\"" << linkPath << "\""
                            << "><span style=\" font-size:12pt; text-decoration: underline; color:#0000ff;\">"
                            << linkName
                            << "</span></a></p>";
                htmlCompose << "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                            << data.toStdString()
                            << "</p>\n"
                            << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>\n"
                            << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>\n";
                }
        }
        return htmlCompose.str();
    }
catch (const Xapian::Error &e) {
        std::cerr << e.get_description() << std::endl;
        exit(EXIT_FAILURE);
    }
}
