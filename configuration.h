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

#include <string>
#include <memory>
#include <utility>
#include <tuple>
#include <QSettings>

namespace XDGSearch {
enum class Pool {   DESKTOP
                  , TEMPLATES
                  , PUBLICSHARE
                  , DOCUMENTS
                  , MUSIC
                  , PICTURES
                  , VIDEOS
                  , SOURCES
                  , END };
Pool& operator++(Pool&);

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
std::string toXDGKey(const Pool&);
class Configuration;
class ConfigurationBase;
class cfgDesktop;
class cfgTemplates;
class cfgPublicShare;
class cfgDocuments;
class cfgMusic;
class cfgPictures;
class cfgVideos;
class cfgSources;
class Settings;
}

Q_DECLARE_METATYPE(XDGSearch::Pool)     // macro used from QVariant in MainWindow::populateCBox()

class XDGSearch::ConfigurationBase  {
friend class Configuration;
public:
    virtual ~ConfigurationBase() = default;
    virtual void defaultSettings(const std::string&) = 0;
protected:
    const std::pair<std::string, std::string> getXDGKeysDirPath(const std::string&);
    void addHelperToPool(const std::string&);
    bool isFirstRun();
    bool isPopulatedDB(const Pool&);
    void initSettings();
    void writeSettings(const helperType&);
    void writeSettings(const poolType&);
    const helperType enqueryHelper(const std::string&);
    const poolType   enqueryPool();
    void removeHelper(const std::string&);
    bool askForConfirmation();
    void setAskForConfirmation(bool);
    QStringList getHelpersNameList();
    void saveMainWindowGeometry(const QByteArray&);
    const QByteArray readMainWindowGeometry();
    helperType helpers;
    poolType pools;
    QSettings settings;
};

class XDGSearch::Configuration final {
public:
    Configuration();
    Configuration(const Pool&);
    //const helperType getHelpers() const         { return d ->helpers; }
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
    std::unique_ptr<ConfigurationBase> d;

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
    d = std::unique_ptr<ConfigurationBase>(new Settings);
}

inline
void XDGSearch::ConfigurationBase::addHelperToPool(const std::string& h)
{
    if(!std::get<2>(pools).empty())
        std::get<2>(pools) += ',';
    std::get<2>(pools) += h;
}

#endif // CONFIGURATION

