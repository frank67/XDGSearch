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

#include "indexer.h"
#include <QDirIterator>
#include <QStringList>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <new>      /// used for bad_alloc
#include <future>


XDGSearch::IndexerBase::IndexerBase(QObject* parent, const Pool& p) :    /// initializes conf member with a Configuration object of Pool p type
          QObject(parent)
        , p_value(9.0)
        , p_valueStep(0.)
        , conf(std::unique_ptr<XDGSearch::Configuration>(new XDGSearch::Configuration(p)))
{
    currentPoolSettings = conf ->enqueryPool(); /// retrieves settings of the current pool type
}

XDGSearch::Indexer::Indexer(QObject* parent, const XDGSearch::Pool& p = XDGSearch::Pool::END) :
    d(new IndexerBase(parent, p))
{   /// connects two signals: IndexerBase to Indexer
    QObject::connect(d, &XDGSearch::IndexerBase::progressValue, this, &Indexer::progressValue);
}

XDGSearch::Indexer::~Indexer()
{
    delete d;
}

void XDGSearch::IndexerBase::resetProgressBarValue()
{   /// sets values for the progress bar to its initial values
    p_value = 9.0;
    p_valueStep = 0.;
}

void XDGSearch::IndexerBase::setProgressBarValue()
{   /// determines values useful for the progress bar
    std::istringstream iss(std::get<POOLHELPERS>(currentPoolSettings));   /// stringstream for comma separated helpers list
    double p_valueGrandTotal(0.);

    for(std::string h; std::getline(iss, h, ','); /* null */)   /// for each helper in the stringstream object
        ++p_valueGrandTotal;

    if(p_valueGrandTotal == 1.)
    {
        p_valueStep = 60;
        return;
    }

    if(p_valueGrandTotal)
        p_valueStep = 90 / p_valueGrandTotal;   /// determines progress bar increment step value
}

void XDGSearch::IndexerBase::populateDB()
{
    emit progressValue(p_value);    /// emits signal to change progress bar
    setProgressBarValue();      /// determines values useful for the progress bar

    QTemporaryDir tempDir;      /// provide an auto-remove temporary directory under /tmp
    while(!tempDir.isValid())   /// iterate until has a valid temporary directory name
        ;
    std::istringstream iss(std::get<POOLHELPERS>(currentPoolSettings));   /// stringstream for comma separated helpers list
    std::forward_list<XDGSearch::helperType> poolHelpers;       /// container for each helper of this pool


    const std::string  tmpDirName  =  tempDir.path().toStdString() + "/"
                     , DBName  =  std::get<LOCALPOOLNAME>(currentPoolSettings)
                     , tmpDBName  =  tmpDirName + DBName;       /// provides names for database and temporary database
try {
        /// try to create the temporary database under /tmp
    Xapian::WritableDatabase tmpDB( tmpDBName
                                  , Xapian::DB_CREATE
                                  , Xapian::DB_BACKEND_GLASS);
    for(std::string h; std::getline(iss, h, ','); /* null */)   /// for each helper in the stringstream object
        poolHelpers.push_front(conf ->enqueryHelper(h));        /// populate the container that holds items informations

    for(const auto& h : poolHelpers)    {   /// for each helper initializes a thread that's added to the container
        if(std::get<HELPERNAME>(h).empty())          /// dummy checks, eventually skips empty helper
            continue;
        const std::string& stopWordsFile = "./stopwords/" + std::get<STOPWORDSFILE>(currentPoolSettings);

        Xapian::Stem stemmer(std::get<STEMMING>(currentPoolSettings));
        Xapian::TermGenerator indexer;
        Xapian::SimpleStopper sstopper;
        std::fstream ifs(stopWordsFile);

        indexer.set_stemmer(stemmer);
        if(ifs) {       /// if the user set the stopwords file populate the simplestopper object
            for(std::string s; std::getline(ifs, s); /* null */)
                sstopper.add(s);
            indexer.set_stopper(&sstopper);
        }

        std::istringstream iss(std::get<EXTENSIONS>(h));
        QStringList filesExtensionFilter;   /// define a QT string list container that'll act as file extension filter

        for(std::string e; std::getline(iss, e, ','); ) {
            e = "*." + e;       /// prepare the string to add to the container, e.g.: *.pdf
            filesExtensionFilter  << QString::fromStdString(e); /// populate the QStringList container
        }
        /// define a file iterator that:
        QDirIterator dirIt( QString::fromStdString(std::get<POOLDIRPATH>(currentPoolSettings))    /// reads from the pool directory and so on
                          , filesExtensionFilter        /// evaluate only these file extensions
                          , QDir::Files                 /// consider only files
                          , QDirIterator::Subdirectories | QDirIterator::FollowSymlinks );  /// recurse sub-directory and evaluate sym-links

        while(dirIt.hasNext())  {   /// outer loop: until file iterator reaches the end
            const std::string& fileFullPathName = dirIt.next().toStdString();    /// fully qualified file name

            auto ftr = std::async( XDGSearch::forEachFile
                                                       , fileFullPathName
                                                       , h );
            const auto& pathAndCmdOutput = ftr.get();

            unsigned int  linescounter = 0;
            const unsigned int granularity = std::get<GRANULARITY>(h)
                             , maxLinesCounted = granularity;

            std::istringstream iss;
            iss.str(pathAndCmdOutput.second);         /// recycle iss object, now holds the command standard output

            for(std::string line, para; !iss.eof(); /* null */ ) { /// inner loop: for each line of the command's standard output
                getline(iss, line);
                if(!iss.eof() && line.empty())      /// avoids empty lines
                    continue;
                if(!granularity)    {   /// if the user set to 0 then only first 15 lines are read, else...
                    if (iss.eof() || linescounter == 15) {

                        if(iss.eof())   {   /// if the eof is reached before the 15° line
                            para += '\n';
                            para += line;
                        }

                        Xapian::Document doc;   /// defines an empty document
                        doc.set_data(para);     /// stores a 15 lines only paragraph into the document
                        /// see: https://trac.xapian.org/wiki/FAQ/UniqueIds
                        doc.add_term("P" + pathAndCmdOutput.first);   /// add fully qualified file name as "P" terms to the document

                        indexer.set_document(doc);
                        indexer.index_text(para);
                        tmpDB.add_document(doc);  /// add the document to the database
                        break;      /// job done for this standard output, breaks in order to process a new one

                    } else {
                        if(!para.empty())
                            para += '\n';    /// if not empty then adds a new line

                        para += line;       /// adds the line to the paragraph
                        ++linescounter;     /// increment the counter
                    }
                } else {        /// ...else standard output will be split into blocks of granularity size
                    if (iss.eof() || linescounter == maxLinesCounted) {

                        if(iss.eof())   {
                            para += '\n';
                            para += line;
                        }

                        Xapian::Document doc;
                        doc.set_data(para);
                        doc.add_term("P" + pathAndCmdOutput.first);

                        indexer.set_document(doc);
                        indexer.index_text(para);

                        tmpDB.add_document(doc);

                        para.clear();       /// reset the paragraph
                        linescounter = 0;   /// reset the counter

                    } else {
                        if(!para.empty())
                            para += '\n';

                        para += line;
                        ++linescounter;
                    }
                }
            }
        }
    }

    tmpDB.commit();
    tmpDB.compact(DBName);    /// writes compacted database from /tmp to $HOME/.local/share/XDGSearch/xdgsearch
    tmpDB.close();

    emit progressValue(100);
    resetProgressBarValue();    /// set initial value when the database build process gets completed
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

void XDGSearch::IndexerBase::forEachHelper(const XDGSearch::helperType& h
                              , const XDGSearch::poolType& currentPoolSettings
                              , Xapian::WritableDatabase* tmpDB)
{   /// threaded function to populate each pool's database
    // static std::unique_ptr<std::mutex> mtx1(new std::mutex);    /// defines mutex to grant mutually exclusive rights to the threads that write the database
    const std::string& stopWordsFile = "./stopwords/" + std::get<STOPWORDSFILE>(currentPoolSettings);

    //Xapian::WritableDatabase inRamDB(std::string(""), Xapian::DB_BACKEND_INMEMORY); /// open the database that'll hold data
    Xapian::Stem stemmer(std::get<STEMMING>(currentPoolSettings));
    Xapian::TermGenerator* indexer;
    Xapian::SimpleStopper sstopper;
    std::fstream ifs(stopWordsFile);

    indexer ->set_stemmer(stemmer);
    if(ifs) {       /// if the user set the stopwords file populate the simplestopper object
        for(std::string s; std::getline(ifs, s); /* null */)
            sstopper.add(s);
        indexer ->set_stopper(&sstopper);
    }

    std::istringstream iss(std::get<EXTENSIONS>(h));
    QStringList filesExtensionFilter;   /// define a QT string list container that'll act as file extension filter

    for(std::string e; std::getline(iss, e, ','); ) {
        e = "*." + e;       /// prepare the string to add to the container, e.g.: *.pdf
        filesExtensionFilter  << QString::fromStdString(e); /// populate the QStringList container
    }
    /// define a file iterator that:
    QDirIterator dirIt( QString::fromStdString(std::get<POOLDIRPATH>(currentPoolSettings))    /// reads from the pool directory and so on
                      , filesExtensionFilter        /// evaluate only these file extensions
                      , QDir::Files                 /// consider only files
                      , QDirIterator::Subdirectories | QDirIterator::FollowSymlinks );  /// recurse sub-directory and evaluate sym-links

    while(dirIt.hasNext())  {   /// outer loop: until file iterator reaches the end
        const std::string& fileFullPathName = dirIt.next().toStdString();    /// fully qualified file name

        auto ftr = std::async( XDGSearch::forEachFile
                                                   , fileFullPathName
                                                   , h );
        const auto& pathAndCmdOutput = ftr.get();

        unsigned int  linescounter = 0;
        const unsigned int granularity = std::get<GRANULARITY>(h)
                         , maxLinesCounted = granularity;

        std::istringstream iss;
        iss.str(pathAndCmdOutput.second);         /// recycle iss object, now holds the command standard output

        for(std::string line, para; !iss.eof(); /* null */ ) { /// inner loop: for each line of the command's standard output
            getline(iss, line);
            if(!iss.eof() && line.empty())      /// avoids empty lines
                continue;
            if(!granularity)    {   /// if the user set to 0 then only first 15 lines are read, else...
                if (iss.eof() || linescounter == 15) {

                    if(iss.eof())   {   /// if the eof is reached before the 15° line
                        para += '\n';
                        para += line;
                    }

                    Xapian::Document doc;   /// defines an empty document
                    doc.set_data(para);     /// stores a 15 lines only paragraph into the document
                    /// see: https://trac.xapian.org/wiki/FAQ/UniqueIds
                    doc.add_term("P" + pathAndCmdOutput.first);   /// add fully qualified file name as "P" terms to the document

                    indexer ->set_document(doc);
                    indexer ->index_text(para);
                    tmpDB ->add_document(doc);  /// add the document to the database
                    break;      /// job done for this standard output, breaks in order to process a new one

                } else {
                    if(!para.empty())
                        para += '\n';    /// if not empty then adds a new line

                    para += line;       /// adds the line to the paragraph
                    ++linescounter;     /// increment the counter
                }
            } else {        /// ...else standard output will be split into blocks of granularity size
                if (iss.eof() || linescounter == maxLinesCounted) {

                    if(iss.eof())   {
                        para += '\n';
                        para += line;
                    }

                    Xapian::Document doc;
                    doc.set_data(para);
                    doc.add_term("P" + pathAndCmdOutput.first);

                    indexer ->set_document(doc);
                    indexer ->index_text(para);
                    tmpDB ->add_document(doc);

                    para.clear();       /// reset the paragraph
                    linescounter = 0;   /// reset the counter

                } else {
                    if(!para.empty())
                        para += '\n';

                    para += line;
                    ++linescounter;
                }
            }
        }
    }
/*
    inRamDB.commit();   /// syncing

    mtx1 ->lock();  /// grants exclusive access to each thread for the following code

    for( Xapian::PostingIterator it = inRamDB.postlist_begin(std::string())
       ; it != inRamDB.postlist_end(std::string())
       ; ++it )
       { tmpDB ->add_document(inRamDB.get_document(*it)); } /// copy document from RAM DB to hard disk temporary DB

    tmpDB ->commit();
    inRamDB.close();

    mtx1 ->unlock();    /// ends of the mutex block*/
}

std::pair<std::string, std::string> XDGSearch::forEachFile(const std::string& fileFullPathName
                                       , const XDGSearch::helperType& h)
{
    const std::string cmd = std::get<COMMANDLINE>(h)         /// start command line with command name
            + " \""                  /// surrounds the arguments with quotation marks: ""
            + fileFullPathName
            + "\"";

    std::string cmdStdOut;      /// container that'll hold the command's standard output

    FILE* pipe = popen(cmd.c_str(), "r");   /// runs the command
    /// may returns this error message: "Syntax error: EOF in backquote substitution"
    /// if so the filename (fileFullPathName) may contains characters that they needed to be escaped
    /// like backtick ` FYI

    for(char buffer[512]; !feof(pipe); /* null */)
        if (fgets(buffer, 512, pipe) != NULL)
            cmdStdOut += buffer;        /// it reads the command standard output

    pclose(pipe);

    const auto& retval = std::make_pair(fileFullPathName, cmdStdOut);
    return retval;

/*
    Xapian::WritableDatabase* inRamDB;
    Xapian::TermGenerator* indexer;

    unsigned int  linescounter = 0;
    const unsigned int granularity = std::get<GRANULARITY>(h)
                     , maxLinesCounted = granularity;

    std::istringstream iss;
    iss.str(cmdStdOut);         /// recycle iss object, now holds the command standard output

    for(std::string line, para; !iss.eof(); /* null */ /*) { /// inner loop: for each line of the command's standard output
        getline(iss, line);
        if(!iss.eof() && line.empty())      /// avoids empty lines
            continue;
        if(!granularity)    {   /// if the user set to 0 then only first 15 lines are read, else...
            if (iss.eof() || linescounter == 15) {

                if(iss.eof())   {   /// if the eof is reached before the 15° line
                    para += '\n';
                    para += line;
                }

                Xapian::Document doc;   /// defines an empty document
                doc.set_data(para);     /// stores a 15 lines only paragraph into the document
                /// see: https://trac.xapian.org/wiki/FAQ/UniqueIds
                doc.add_term("P" + fileFullPathName);   /// add fully qualified file name as "P" terms to the document

                indexer->set_document(doc);
                indexer->index_text(para);
                inRamDB->add_document(doc);  /// add the document to the database
                break;      /// job done for this standard output, breaks in order to process a new one

            } else {
                if(!para.empty())
                    para += '\n';    /// if not empty then adds a new line

                para += line;       /// adds the line to the paragraph
                ++linescounter;     /// increment the counter
            }
        } else {        /// ...else standard output will be split into blocks of granularity size
            if (iss.eof() || linescounter == maxLinesCounted) {

                if(iss.eof())   {
                    para += '\n';
                    para += line;
                }

                Xapian::Document doc;
                doc.set_data(para);
                doc.add_term("P" + fileFullPathName);

                indexer->set_document(doc);
                indexer->index_text(para);
                inRamDB->add_document(doc);

                para.clear();       /// reset the paragraph
                linescounter = 0;   /// reset the counter

            } else {
                if(!para.empty())
                    para += '\n';

                para += line;
                ++linescounter;
            }
        }
    }*/
}

const Xapian::MSet XDGSearch::IndexerBase::enqueryDB(const std::string& query_string) const
{
try {
    /// Open the database for searching.
    Xapian::Database db(std::get<LOCALPOOLNAME>(currentPoolSettings));

    /// Start an enquire session.
    Xapian::Enquire enquire(db);

    /// Parse the query string to produce a Xapian::Query object.
    Xapian::QueryParser qp;
    Xapian::Stem stemmer(std::get<STEMMING>(currentPoolSettings));
    qp.set_stemmer(stemmer);
    qp.set_database(db);
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    Xapian::Query query = qp.parse_query(query_string);

    /// Find the top 10 results for the query.
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
        /// write HTML header
        composeHTML << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
                    << "<html><head><meta name=\"qrichtext\" content=\"1\" /><title>Result</title><style type=\"text/css\">\n"
                    << "p, li { white-space: pre-wrap; }\n"
                    << "</style></head><body style=\" font-family:'Sans Serif'; font-size:9pt; font-weight:400; font-style:normal;\">\n";

        if(!matches.size()) /// if no result then matches.size() == 0 therefore set composeHTML to show: "No items found"
            composeHTML  << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>"
                         << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>"
                         << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>"
                         << "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /></p>"
                         << "<p align=\"center\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:12pt; font-weight:600; color:#bababa;\">No items found</span></p></body></html>";
        else    {           /// matches has documents, starts formatting
            for( Xapian::MSetIterator matchesIterator = matches.begin()
               ; matchesIterator != matches.end()
               ; ++matchesIterator) {
                auto term_iterator = matchesIterator.get_document().termlist_begin();
                /// advance until find the term added to the document with add_term (the path of the file)
                while(*(*term_iterator).cbegin() != 'P')
                    ++term_iterator;
                /// the iterator now points to the fully qualified file name that holds the document
                std::istringstream iss(*term_iterator);
                std::string   linkName
                            , linkPath
                            , paragraphDocument;
                while(std::getline(iss, linkName, '/')) /// iteration in order to achieve the file URI e.g.:
                    if(linkName == "P")                 /// "file:///foo/bar/baz.pdf" and the file name e.g.:
                        linkPath = "file://";           /// "baz.pdf" both useful when compose the HTML formatted answer to the query
                    else
                        linkPath = linkPath + "/" + linkName;
                /// retrieve the data of the document pointed by the iterator
                QString documentText = QString::fromStdString(matchesIterator.get_document().get_data());

                documentText.replace("&","&amp;");  /// in documentText replaces HTML reserved characters
                documentText.replace("<","&lt;");
                documentText.replace(">","&gt;");
                iss.clear();
                iss.str(soughtTerms);       /// recycle iss object now it holds the terms to search
                /// iteration to achieve bolded sought terms effect when documentText will be added to composeHTML
                for( std::string term
                   ; iss >> term
                   ; /* null */)
                {
                    for( decltype(documentText.indexOf(QString(""))) pos = documentText.indexOf(QString::fromStdString(term), 0, Qt::CaseInsensitive)
                       ; pos != -1
                       ; /* null */)
                    {
                        documentText.insert(pos, "<span style=\" font-weight:600;\">"); /// add bold effect syntax
                        pos = documentText.indexOf( QString::fromStdString(term)
                                                  , pos
                                                  , Qt::CaseInsensitive); /// position at the term is located
                        pos += term.size();     /// advance pos cursor of the term size
                        documentText.insert(pos, "</span>");    /// now pos it's after the term so insert the syntax needed for the bold effect
                        ++pos;  /// advance pos cursor so if there is another identical term will be located from the next statement
                        pos = documentText.indexOf( QString::fromStdString(term)
                                                  , pos
                                                  , Qt::CaseInsensitive);   /// looks for term again, if not found pos == -1 therefore quitting this loop
                    }
                }
                iss.clear();
                iss.str(documentText.toStdString());
                for(std::string line; std::getline(iss, line); /* null */)
                {
                    paragraphDocument += "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                                      + line + "</p>";
                }
                composeHTML << "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><a href="
                /// surrounds linkPath with quotation marks so it'll be legal also if it contains white spaces
                            << "\"" << linkPath << "\""
                            << "><span style=\" font-size:12pt; text-decoration: underline; color:#0000ff;\">"
                            << linkName
                            << "</span></a></p>";
                composeHTML << "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                            << paragraphDocument
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
