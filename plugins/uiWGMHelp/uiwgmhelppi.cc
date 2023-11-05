#include "odplugin.h"
#include "uimsg.h"
#include "uihelpview.h"
#include "uiwgmhelpmod.h"
#include "wmplugins.h"

typedef struct keyholder {
    const char* key;
    const char* link;
} keyholder;

keyholder keys[] = {
    {"avo", "avoattrib/"},
    {"avop","avopolarattrib/"},
    {"deh", "wmtools/#data-extent-horizon"},
    {"ext", "externalattrib/"},
    {"fpl", "wmtools/#fault-surface-3d-horizon-intersection-polyline"},
    {"geodisp","geopackagedisplay/"},
    {"geopkg", "geopackageexport/"},
    {"geotiff", "geotiffexport/"},
    {"grad", "gradientattrib/"},
    {"grid2d3d","grid2d-3d/"},
    {"ltf", "localattrib/"},
    {"mistie", "mistie/"},
    {"mlv", "mlvfilter/"},
    {"pconz","wmtools/#constant-z-polyline"},
    {"pcvx","mwtools/#convex-hull-polygon"},
    {"rspec","rspecattrib/"},
    { 0, 0}
};

class WGMHelpProvider : public SimpleHelpProvider
{
public:
    WGMHelpProvider( const char* baseurl )
    : SimpleHelpProvider(baseurl)
    {}

    static void initClass()
    {
        HelpProvider::factory().addCreator( WGMHelpProvider::createInstance, "wgm");
    }

    static HelpProvider* createInstance()
    {
        BufferString baseurl( "http://waynegm.github.io/WMPlugin-Docs/docs/plugins/" );
        WGMHelpProvider* hp = new WGMHelpProvider( baseurl.buf());
        int idx = 0;
        while (keys[idx].key) {
            hp->addKeyLink( keys[idx].key, keys[idx].link );
            idx++;
        }
        return hp;
    }
};

mDefODPluginInfo(uiWGMHelp)
{
    mDefineStaticLocalObject( PluginInfo, retpi,(
	"uiWGMHelp plugin",
	wmPlugins::sKeyWMPlugins(),
	wmPlugins::sKeyWMPluginsAuthor(),
	wmPlugins::sKeyWMPluginsVersion(),
	"Help provider for the WMPlugin's collection.") );
    return &retpi;
}

mDefODInitPlugin(uiWGMHelp)
{
    WGMHelpProvider::initClass();
    return 0;
}
