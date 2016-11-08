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

#ifndef CONFIGURATION
#define CONFIGURATION

#include <memory>
#include <utility>
#include <tuple>
#include <QSettings>

namespace XDGSearch {                   // open the application namespace
enum class Pool {   DESKTOP             // define an enum class to hold the XDG keys that give name to the directories
                  , TEMPLATES
                  , PUBLICSHARE
                  , DOCUMENTS
                  , MUSIC
                  , PICTURES
                  , VIDEOS
                  , SOURCES
                  , END };
Pool& operator++(Pool&);                // declare prefix increment operator overloaded for the Pool type

using poolType = std::tuple<  std::string       //  0 PoolName
                            , std::string       //  1 localPoolName
                            , std::string       //  2 PoolHelpers
                            , std::string       //  3 PoolDirPath
                            , std::string       //  4 stemming
                            , std::string>;     //  5 stopwordsfile

using helperType = std::tuple<std::string       //  0 helper name
                            , std::string       //  1 extensions
                            , std::string       //  2 command line
                            , unsigned int>;    //  3 granularity
std::string toXDGKey(const Pool&);              // translate from Pool type item to string name key
class Configuration;                    // Interface class for configuration/settings  operation
class ConfigurationBase;                // "Cheshire Cat" implemention class for Configuration class
class cfgDesktop;                       // Specialized classes that inherit from ConfigurationBase to define its pure virtual funcion
class cfgTemplates;
class cfgPublicShare;
class cfgDocuments;
class cfgMusic;
class cfgPictures;
class cfgVideos;
class cfgSources;
class Settings;                         // Class used from Configuration when it has to deal with settings operations
}

Q_DECLARE_METATYPE(XDGSearch::Pool)     // macro used from QVariant in MainWindow::populateCBox()

class XDGSearch::ConfigurationBase  {
friend class Configuration;
public:
    virtual ~ConfigurationBase() = default;
    virtual void defaultSettings(const std::string&) = 0;
private:
    bool isFirstRun();                                      // check if "askQuitConfirmation" entry exist in the .conf file
    bool isPopulatedDB(const Pool&);                        // check if the object build from the specified pool has an already existing database
    void writeSettings(const helperType&);                  // write to .conf file contents of the tuple object
    void writeSettings(const poolType&);                    // write to .conf file contents of the tuple object
    const helperType enqueryHelper(const std::string&);     // return a tuple reading data from the .conf file
    const poolType   enqueryPool();                         // return a tuple reading data from the .conf file
    void removeHelper(const std::string&);                  // remove all entries for the specified helper name in .conf file
    bool askForConfirmation();                              // query .conf file "askQuitConfirmation" entry
    void setAskForConfirmation(bool);                       // set "askQuitConfirmation" .conf file entry
    QStringList getHelpersNameList();                       // query .conf file for the helpers list
    void saveMainWindowGeometry(const QByteArray&);         // set geometry and window position in .conf file
    const QByteArray readMainWindowGeometry();              // query geometry and window position in .conf file
protected:
    const std::pair<std::string, std::string> getXDGKeysDirPath(const std::string&);    // retrive the XDG directory path by quering a key in the home directory XDG configuration file
    void addHelperToPool(const std::string&);               // add specified helper to private pools tuple object defined in this class
    void initSettings();                                    // restore default settings
    poolType pools;
    QSettings settings;
};

class XDGSearch::Configuration final {
public:
    Configuration();
    Configuration(const Pool&);

    const poolType getPools() const             { return d ->pools; }
    void defaultSettings(const std::string& s) const    { d ->defaultSettings(s); }
    void addHelperToPool(const std::string& h) const    { d ->addHelperToPool(h); }
    const helperType enqueryHelper(const std::string& h) const      { return d ->enqueryHelper(h); }
    const poolType   enqueryPool() const        { return d ->enqueryPool(); }
    void removeHelper(const std::string& h) const   { d ->removeHelper(h); }
    bool askForConfirmation() const     { return d ->askForConfirmation(); }
    void setAskForConfirmation(bool b) const    { d ->setAskForConfirmation(b); }
    bool isFirstRun() const     { return d ->isFirstRun(); }
    bool isPopulatedDB(const Pool& p) const     { return d ->isPopulatedDB(p); }
    void initSettings() const   { return d ->initSettings(); }
    void writeSettings(const helperType& ht) const   { return d ->writeSettings(ht); }
    void writeSettings(const poolType& pt) const     { return d ->writeSettings(pt); }
    QStringList getHelpersNameList() const      { return d ->getHelpersNameList(); }
    void saveMainWindowGeometry(const QByteArray& g) const       { d ->saveMainWindowGeometry(g); }
    const QByteArray readMainWindowGeometry() const     { return d ->readMainWindowGeometry(); }
private:
    std::unique_ptr<XDGSearch::ConfigurationBase> d;
};

class XDGSearch::cfgDesktop final : public ConfigurationBase {
    void defaultSettings(const std::string&) override;
};

class XDGSearch::cfgTemplates final : public ConfigurationBase {
    void defaultSettings(const std::string&) override;
};

class XDGSearch::cfgPublicShare final : public ConfigurationBase {
    void defaultSettings(const std::string&) override;
};

class XDGSearch::cfgDocuments final : public ConfigurationBase {
    void defaultSettings(const std::string&) override;
};

class XDGSearch::cfgMusic final : public ConfigurationBase {
    void defaultSettings(const std::string&) override;
};

class XDGSearch::cfgPictures final : public ConfigurationBase {
    void defaultSettings(const std::string&) override;
};

class XDGSearch::cfgVideos final : public ConfigurationBase {
    void defaultSettings(const std::string&) override;
};

class XDGSearch::cfgSources final : public ConfigurationBase {
    void defaultSettings(const std::string&) override;
};

class XDGSearch::Settings final : public ConfigurationBase {
    void defaultSettings(const std::string&) override;
};

inline
XDGSearch::Configuration::Configuration()
{
    d = std::unique_ptr<XDGSearch::ConfigurationBase>(new Settings);
}

#endif // CONFIGURATION

