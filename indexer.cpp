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


XDGSearch::IndexerBase::IndexerBase(const Pool& p) :    // initializes conf member with a Configuration object of Pool p type
        conf(std::unique_ptr<XDGSearch::Configuration>(new XDGSearch::Configuration(p)))
{
    currentPoolSettings = conf ->enqueryPool(); // retrieves settings of the current pool type
}

void XDGSearch::IndexerBase::populateDB() const
{
    QTemporaryDir tempDir;      // provide an auto-remove temporary directory under /tmp
    while(!tempDir.isValid())   // iterate until has a valid temporary directory name
        ;
    std::istringstream iss(std::get<2>(currentPoolSettings));   // stringstream for comma separated helpers list
    std::forward_list<XDGSearch::helperType> poolHelpers;       // container for each helper of this pool
    std::forward_list<std::unique_ptr<std::thread>> helpersThread;  // container for threaded execution for each helper

    const std::string  tmpDirName  =  tempDir.path().toStdString() + "/"
                     , DBName  =  std::get<1>(currentPoolSettings)
                     , tmpDBName  =  tmpDirName + DBName;       // provides names for database and temporary database
try {
        // try to create the temporary database under /tmp
    Xapian::WritableDatabase* tmpDB(new Xapian::WritableDatabase( tmpDBName
                                                                , Xapian::DB_CREATE
                                                                , Xapian::DB_BACKEND_CHERT) );
    for(std::string h; std::getline(iss, h, ','); /* null */)   // for each helper in the stringstream object
        poolHelpers.push_front(conf ->enqueryHelper(h));        // populate the container that holds items informations

    for(const auto& h : poolHelpers)    {   // for each helper initializes a thread that's added to the container
        if(std::get<0>(h).empty())          // dummy checks, eventually skips empty helper
            continue;
        helpersThread.push_front(std::unique_ptr<std::thread>(new std::thread( forEachHelper
                                                                             , h
                                                                             , currentPoolSettings
                                                                             , tmpDB) ));
    }
    for(auto& t : helpersThread)
        t ->join();     // join each thread object stored in the container

    tmpDB ->compact(DBName);    // writes compacted database from /tmp to $HOME/.local/share/XDGSearch/xdgsearch
    tmpDB ->close();
    delete tmpDB;               // frees resources
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
    static std::unique_ptr<std::mutex> mtx1(new std::mutex);    // defines mutex to grant mutually exclusive rights to the threads that write the database
    const std::string stopWordsFile = "./stopwords/" + std::get<5>(currentPoolSettings);

    Xapian::WritableDatabase inRamDB(Xapian::InMemory::open()); // open the database that'll hold data
    Xapian::Stem stemmer(std::get<4>(currentPoolSettings));
    Xapian::TermGenerator indexer;
    Xapian::SimpleStopper sstopper;
    std::fstream ifs(stopWordsFile);

    indexer.set_stemmer(stemmer);
    if(ifs) {       // if the user set the stopwords file populate the simplestopper object
        for(std::string s; std::getline(ifs, s); /* null */)
            sstopper.add(s);
        indexer.set_stopper(&sstopper);
    }

    std::istringstream iss(std::get<1>(h));
    QStringList filesExtensionFilter;   // define a QT string list container that'll act as file extension filter

    for(std::string e; std::getline(iss, e, ','); ) {
        e = "*." + e;       // prepare the string to add to the container, e.g.: *.pdf
        filesExtensionFilter  << QString::fromStdString(e); // populate the QStringList container
    }
    // define a file iterator that:
    QDirIterator dirIt( QString::fromStdString(std::get<3>(currentPoolSettings))    // reads from the pool directory and so on
                      , filesExtensionFilter        // evaluate only these file extensions
                      , QDir::Files                 // consider only files
                      , QDirIterator::Subdirectories | QDirIterator::FollowSymlinks );  // recurse sub-directory and evaluate sym-links

    while(dirIt.hasNext())  {   // outer loop: until file iterator reaches the end
        const std::string  fileFullPathName = dirIt.next().toStdString()    // fully qualified file name
                         , cmd = std::get<2>(h)         // start command line with command name
                               + " \""                  // surrounds the arguments with quotation marks: ""
                               + fileFullPathName
                               + "\"";
        std::string cmdStdOut;      // container that'll hold the command's standard output

        FILE* pipe = popen(cmd.c_str(), "r");   // runs the command

        for(char buffer[512]; !feof(pipe); /* null */)
            if (fgets(buffer, 512, pipe) != NULL)
                cmdStdOut += buffer;        // it reads the command standard output

        pclose(pipe);

        unsigned int  linescounter = 0;
        const unsigned int granularity = std::get<3>(h)
                         , maxLinesCounted = granularity;
        iss.clear();
        iss.str(cmdStdOut);         // recycle iss object, now holds the command standard output

        for(std::string line, para; !iss.eof(); /* null */) { // inner loop: for each line of the command's standard output
            getline(iss, line);
            if(!iss.eof() && line.empty())      // avoids empty lines
                continue;
            if(!granularity)    {   // if the user set to 0 then only first 15 lines are read, else...
                if (iss.eof() || linescounter == 15) {

                    if(iss.eof())   {   // if the eof is reached before the 15° line
                        para += ' ';
                        para += line;
                    }

                    Xapian::Document doc;   // defines an empty document
                    doc.set_data(para);     // stores a 15 lines only paragraph into the document
                    // see: https://trac.xapian.org/wiki/FAQ/UniqueIds
                    doc.add_term("P" + fileFullPathName);   // add fully qualified file name as "P" terms to the document

                    indexer.set_document(doc);
                    indexer.index_text(para);
                    inRamDB.add_document(doc);  // add the document to the database
                    break;      // job done for this standard output, breaks in order to process a new one

                } else {
                    if(!para.empty())
                        para += ' ';    // if not empty then adds a single space

                    para += line;       // adds the line to the paragraph
                    ++linescounter;     // increment the counter
                }
            } else {        // ...else standard output will be split into blocks of granularity size
                if (iss.eof() || linescounter == maxLinesCounted) {

                    if(iss.eof())   {
                        para += ' ';
                        para += line;
                    }

                    Xapian::Document doc;
                    doc.set_data(para);
                    doc.add_term("P" + fileFullPathName);

                    indexer.set_document(doc);
                    indexer.index_text(para);
                    inRamDB.add_document(doc);

                    para.clear();       // reset the paragraph
                    linescounter = 0;   // reset the counter

                } else {
                    if(!para.empty())
                        para += ' ';

                    para += line;
                    ++linescounter;
                }
            }
        }
    }
    inRamDB.commit();   // syncing

    mtx1 ->lock();  // grants exclusive access to each thread for the following code

    for( Xapian::PostingIterator it = inRamDB.postlist_begin(std::string())
       ; it != inRamDB.postlist_end(std::string())
       ; ++it )
       { tmpDB ->add_document(inRamDB.get_document(*it)); } // copy document from RAM DB to hard disk temporary DB

    tmpDB ->commit();
    inRamDB.close();

    mtx1 ->unlock();    // ends of the mutex block
}

const Xapian::MSet XDGSearch::IndexerBase::enqueryDB(const std::string& query_string) const
{
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

std::string XDGSearch::IndexerBase::queryResult::composeResult(const Xapian::MSet& matches)
{
try {
        std::ostringstream composeHTML;
        // write HTML header
        composeHTML << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                    << "<html><head><meta name=\"qrichtext\" content=\"1\" /><title>Result</title><style type=\"text/css\">\n"
                    << "p, li { white-space: pre-wrap; }\n"
                    << "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n";

        if(!matches.size()) // if no result matches.size() == 0 therefore set composeHTML to show: "Not found"
            composeHTML  << "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Not found</p></body></html>";
        else    {           // matches has documents, starts formatting
            for( Xapian::MSetIterator matchesIterator = matches.begin()
               ; matchesIterator != matches.end()
               ; ++matchesIterator) {
                auto term_iterator = matchesIterator.get_document().termlist_begin();
                // advance until find the term added to the document with add_term (the path of the file)
                while(*(*term_iterator).cbegin() != 'P')
                    ++term_iterator;
                // the iterator now points to the fully qualified file name that holds the document
                std::istringstream iss(*term_iterator);
                std::string   linkName
                            , linkPath;
                while(std::getline(iss, linkName, '/')) // iteration in order to achieve the file URI e.g.:
                    if(linkName == "P")                 // "file:///foo/bar/baz.pdf" and the file name e.g.:
                        linkPath = "file://";           // "baz.pdf" both useful when compose the HTML formatted answer to the query
                    else
                        linkPath = linkPath + "/" + linkName;
                // retrieve the data of the document pointed by the iterator
                QString documentText = QString::fromStdString(matchesIterator.get_document().get_data());
                iss.clear();
                iss.str(soughtTerms);       // recycle iss object now it holds the terms to search
                // iteration to achieve bolded sought terms effect when documentText will be added to composeHTML
                for( std::string term
                   ; iss >> term
                   ; /* null */)
                    for( int pos = documentText.indexOf(QString::fromStdString(term), 0, Qt::CaseInsensitive)
                       ; pos != -1
                       ; /* null */)   {
                        documentText.insert(pos, "<span style=\" font-weight:600;\">"); // add bold effect syntax
                        pos = documentText.indexOf( QString::fromStdString(term)
                                                  , pos
                                                  , Qt::CaseInsensitive); // position at the term is located
                        pos += term.size();     // advance pos cursor of the term size
                        documentText.insert(pos, "</span>");    // now pos it's after the term so insert the syntax needed for the bold effect
                        ++pos;  // advance pos cursor so if there is another identical term will be located from the next statement
                        pos = documentText.indexOf( QString::fromStdString(term)
                                                  , pos
                                                  , Qt::CaseInsensitive);   // looks for term again, if not found pos == -1 therefore quitting this loop
                    }

                composeHTML << "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href="
                // surrounds linkPath with quotation marks so it'll be legal also if it contains white spaces
                            << "\"" << linkPath << "\""
                            << "><span style=\" font-size:12pt; text-decoration: underline; color:#0000ff;\">"
                            << linkName
                            << "</span></a></p>";
                composeHTML << "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                            << documentText.toStdString()
                            << "</p>\n"
                            << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>\n"
                            << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>\n";
                }
        }
        return composeHTML.str();
    }
catch (const Xapian::Error &e) {
        std::cerr << e.get_description() << std::endl;
        exit(EXIT_FAILURE);
    }
}
