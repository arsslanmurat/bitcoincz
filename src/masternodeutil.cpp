#include "masternodeutil.h"
#include "compat.h"
#include "tinyformat.h"
#include "utiltime.h"
#include "amount.h"

#include "base58.h"
#include "init.h"
#include "netbase.h"
#include "main.h"
#include "util.h"
#include "wallet.h"

#include <univalue.h>
#include <boost/filesystem/path.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/thread/exceptions.hpp>
#include <exception>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>
#include <fstream>

#include "activemasternode.h"
#include "masternode-payments.h"
#include "masternode-sync.h"
#include "masternodeconfig.h"
#include "masternodeman.h"
#include "rpc/server.h"
#include "utilmoneystr.h"

std::string makeGenkey()
{
    //genkey
    CKey secret;
    secret.MakeNewKey(false);

    auto mnGenkey = CBitcoinSecret(secret).ToString();

    return mnGenkey;
}

void writeConfFile(std::string  _line)
{
    FILE *  fileout=NULL;
    
    boost::filesystem::path pathDebug = GetDataDir() / "bcz.conf";

    fileout = fopen (pathDebug.string().c_str(),"aw");
    std::string  s =std::string ("\n")+_line;

    fprintf(fileout,"%s",s.c_str());

    fclose (fileout);
}

std::string getConfParam(std::string _arg)
{
    for (auto ar : mapArgs)
    {
        if(ar.first==_arg)
            return ar.second;    
    }
    return std::string ("");
}

//CBitcoinAddress GetAccountAddress( std::string  strAccount, bool bForceNew=false);

void writeMasternodeConfFile(std::string  _alias, std::string  _ipport,std::string  mnprivkey,std::string  _output,std::string  _index)
{
    FILE *  fileout=NULL;
    boost::filesystem::path pathDebug2 = GetDataDir() / "masternode.conf";

    fileout = fopen (pathDebug2.string().c_str(),"w");// use "a" for append, "w" to overwrite, previous content will be deleted

    std::string  s =std::string ("\n")+_alias+std::string (" ")+_ipport+std::string (" ")+mnprivkey+std::string (" ")+_output+std::string (" ")+_index;

    fprintf(fileout,"%s",s.c_str());
    fclose (fileout); // must close after opening
}

void writeMasternodeConfInfo(std::string  mnGenkey, std::string  strIpPort)
{
    writeConfFile("masternode=1");
    writeConfFile("masternodeprivkey="+mnGenkey);
    writeConfFile("externalip="+strIpPort);
}

std::vector<std::pair<std::string ,std::string >> checkMasternodeOutputs()
{
    std::vector<std::pair<std::string ,std::string >> result;

    std::vector<COutput> vPossibleCoins;
    pwalletMain->AvailableCoins(vPossibleCoins, true, NULL, false, ONLY_10000);

    UniValue obj(UniValue::VOBJ);
    for (COutput& out : vPossibleCoins)
    {
        result.push_back(std::pair<std::string ,std::string >{out.tx->GetHash().ToString(),std::to_string(out.i)});
    }

    return result;
}

void cleanConf()
{
    boost::filesystem::path pathDebug = GetDataDir() / "bczn.conf";

    std::vector<std::string> lines;

    std::string item_name;
    std::ifstream nameFileout;

    nameFileout.open(pathDebug.string().c_str());

    while (nameFileout >> item_name)
    {
        if(item_name.find("masternode") == std::string::npos
        && item_name.find("externalip") == std::string::npos)
        {
           lines.push_back(item_name);
        }
    }

    nameFileout.close();
    
    remove(pathDebug.string().c_str());
    
    for (std::string& key : lines)
        writeConfFile(key);
}

void RemoveMasternodeConfigs()
{
    cleanConf();
    boost::filesystem::path masternodeConfPath = GetDataDir() / "masternode.conf";

    if (boost::filesystem::exists(masternodeConfPath.string().c_str()))        
        remove(masternodeConfPath.string().c_str());
}

