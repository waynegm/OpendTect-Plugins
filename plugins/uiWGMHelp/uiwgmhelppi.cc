#include "odplugin.h"
#include "uimsg.h"
#include "uihelpview.h"
#include "uiwgmhelpmod.h"

typedef struct keyholder {
    const char* key;
    const char* link;
} keyholder;

keyholder keys[] = {
    {"avo", "AVOAttrib.html"},
    {"ext", "ExternalAttrib.html"},
    {"grad", "GradientAttrib.html"},
    {"ltf", "LTFAttrib.html"},
    {"mlv", "MLVFilter.html"},
    {"rspec","RSpecAttrib.html"},
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
        BufferString baseurl( "http://waynegm.github.io/OpendTect-Plugin-Docs/plugins/" );
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
        "OpendTect",
        "Wayne Mogg",
        "1.0",
        "Help provider for Wayne Moggs attribute plugins.") );
    return &retpi;
}

mDefODInitPlugin(uiWGMHelp)
{
    WGMHelpProvider::initClass();
    return 0;
}
