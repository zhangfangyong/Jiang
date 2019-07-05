#ifndef JIANG_LIBRARY_H
#define JIANG_LIBRARY_H
#include <string>
#include <cstring>
using namespace std;
//
// Created by zfy on 2019/7/3.
//

/**************************************************/
//纾佺洏绠＄悊锛氫綅绀哄浘+杩炵画鍒嗛厤(榛樿鍗曚綅涓築yte)
#define _virtualDiskSize 8*1024*1024    //瀹氫箟铏氭嫙纾佺洏(txt鏂囦欢鍏呭綋)鐨勫ぇ灏忎负8M锛? 鍐欏叆鐨勯兘鏄瓧姣嶇瓑锛屾墍浠ユ瘮杈冨皬锛?
#define _blockSize 1024          //鐩樺潡澶у皬涓?KB锛?
#define _contentSize 1000        //涓€涓狟LOCK鑳藉啓鐨勫瓧鑺傛暟锛?
#define _fcbsSize 15			//涓€涓狟LOCK鑳藉瓨鍌ㄧ殑FCB涓暟
#define _blockCount _virtualDiskSize/_blockSize //铏氭嫙纾佺洏鐨勭洏鍧椾釜鏁?            //8k涓紱  濡傛灉鐢ㄥ瓧绗?/1鏍囪瘑鏄惁鍗犵敤鐩樺潡锛岃繖绉嶄綅绀哄浘琛ㄧず鏂瑰紡锛岄渶瑕?涓紱濡傛灉鐢ㄤ簩杩涘埗浣?/1锛岃〃绀虹殑璇濓紝涓€涓瓧绗︽槸鍏釜浜岃繘鍒朵綅锛屽彧闇€瑕?涓鐩樺潡(鐩墠涓嶇煡閬撳疄鐜版柟寮?锛?
#define _FCB_size 32//64                //FCB瀹氶暱绠＄悊锛?4B锛?/鍒欎竴涓鐩樺潡鍙互瀛樺偍16涓紱
//#define _bitMapCount 8                  //浣嶅紡鍥惧崰鐢ㄧ殑纾佺洏鍧椾釜鏁?
//鍒嗛厤璁惧畾x涓鐩樺潡锛堝悓鏃惰瀹氫簡鏈€澶氭枃浠舵暟锛夊瓨鍌ㄧ洰褰曟爲锛?
//0-7纾佺洏鍧椾綅寮忓浘锛?-127锛?璁や负0-127锛屾槸绯荤粺绌洪棿)鍓╀綑鐨勭鐩樺潡閮界敤鏉ュ瓨鍌ㄦ枃浠跺唴瀹癸紝閫氳繃鎸囬拡鎸囧悜锛?28-8191(鐢ㄦ埛绌洪棿)锛?
#define _systemUsedBlock 128               //绯荤粺浣跨敤鐨刡lock鎬绘暟=FAB琛紙32锛夛紙+瀛樺彇鎺у埗琛級
////#define _FatBlockCount 32                  //浣嶅紡鍥惧崰鐢ㄧ殑纾佺洏鍧椾釜鏁?
////#define _FCB_count 1536                  //FCB鎬讳釜鏁帮紱锛?28-32锛?16
#define _BLOCK_count 8064                    //BLOCK鎬讳釜鏁帮紱

#define _maxusers 21
//鐩綍绠＄悊锛欶AT琛?
struct FAT {
    //fat fats[8*1024];//鎬荤洏鍧楁暟
    int fats[_blockCount];//鍗犳嵁32鐩樺潡
    FAT() {
        for (int i = 0; i < _blockCount; i++) {
            fats[i] = -1;
        }
    }
};


//涓€涓槸32B锛?
struct FCB {
    char _name[20];              //榛樿28B锛?
    //char _spare[12];
    int _type;                 //int 鏄?B锛?
    int _size;
    int _block;//棣栫洏鍧楀彿-鏌ヨ〃鍙緱澶氫釜鐩樺潡
    char _time[10];
    char _access[_maxusers];
    //FCB(int x) {
    FCB() {
        strcpy(this->_name, "");
        strcpy(this->_time, "");
        strcpy(this->_access, "");

        this->_type = 1;                      //榛樿涓虹洰褰曪紝1涓虹洰褰曪紱
        this->_size = 0;//鐩綍鏂囦欢涓篎CB涓暟锛屾枃浠朵负鏂囦欢鎬诲ぇ灏?
        this->_block = -1;
        for (int i = 0; i < _maxusers; i++)
            _access[i] = '0';
        this->_access[0] = '4';//0:无权 1：只读 2：读写 3：删除改名移位复制剪切等全部操作  4：最高权限
        //this->_access[x] = '4';

    }
};

//涓€涓槸32B锛?KB
struct FCBBLOCK {/////大量冗余？？？
    FCB _content[_fcbsSize];//31*32B//15*64
    char _spare[56];                //杩欎釜璺宠繃锛涘浣欑殑锛?
    //char _spare[15];                //杩欎釜璺宠繃锛涘浣欑殑锛?
    int _contentLength;                  //鏂囦欢鍐呭闀垮害锛?
    int _blockID;                      //杩欎釜block鐨処D锛?
    ////BLOCK* _nextBlock;
    FCBBLOCK() {
        for (int i = 0; i < 56; i++)
            _spare[i] = '\0';

        this->_contentLength = 0;
        this->_blockID = -1;
        ////this->_nextBlock = NULL;
    }
};
//1KB
struct FileBLOCK {
    char _content[1001];
    //char _spare[11];                //杩欎釜璺宠繃锛涘浣欑殑锛?
    char _spare[15];                //杩欎釜璺宠繃锛涘浣欑殑锛?
    int _contentLength;                  //鏂囦欢鍐呭闀垮害锛?
    int _blockID;                      //杩欎釜block鐨処D锛?
    ////BLOCK* _nextBlock;
    FileBLOCK() {
        for (int i = 0; i < 15; i++)
            this->_spare[i] = '\0';

        this->_contentLength = 0;
        this->_blockID = -1;
        ////this->_nextBlock = NULL;
    }
};

/*******************************************************/

struct StringList
{
    string content;
    StringList*	next;
    /*
    StringList(string inputString) {
    this->content=inputString ;
    next = NULL;
    }*/
};

struct User {
    //list<FCB> User_List;	//澶氱敤鎴凤紱浣嗘湰绯荤粺鍙疄鐜板崟鐢ㄦ埛
    FCB* _EnterFileSystem;   //鐢ㄦ埛杩涘叆鏂囦欢绯荤粺鐨勫叆鍙ｏ紱
};



extern char* _virtualDiskAddr;   //铏氭嫙纾佺洏鐨勫叆鍙ｅ湴鍧€锛?

extern int _emptyBLOCK_Count;      //鍙敤鐨凚LOCK鏁伴噺锛?
//extern int _emptyFCB_Count;         //鍙敤鐨凢CB鐨勬暟閲忥紱

extern FCB* _current; //褰撳墠鐩綍锛?
extern string current_path; //褰撳墠璺緞锛?

extern FCB* _root;   //鎸囧悜root锛屽疄鐜扮粷瀵硅矾寰勶紱

extern FAT* _fat;//绯荤粺FAT琛?
extern FCBBLOCK* _users;

/**********************************************************************/
//纾佺洏绠＄悊澹版槑锛?
/**********************************************************************/
//鐩綍绠＄悊澹版槑锛?
/**********************************************************************/
/**********************************************************************/
//磁盘管理声明；
/**********************************************************************/
//目录管理声明；
/**********************************************************************/
void sys_initDisk(); //初始化虚拟磁盘；                                      //恢复磁盘测试后才知道是否正确；
FCB* sys_returnFCB(string path_name, string &name, int type, FCB* &parent,int &B,int &F);  //目前完善成功；
int sys_createuser(string username);
int sys_suroot();
int sys_su(string name);
StringList* sys_dir(string dirPath);//查看当前目录内容；				//目前测试成功；
int sys_mkdir(string dirPath);                                         //目前测试成功；
int sys_rmdir(string dirPath);//删除目录;							 //目前测试成功；
int sys_create_file(string filePath);//创建文件；                     //目前测试成功；
int sys_delete_file(string filePath);//删除文件；                       //目前测试成功；
int sys_rename(string path, int type, string new_name);//重命名文件；    //目前测试成功；
int sys_overwrite_file(FCB* file, string content);//向文件写内容；   //目前测试成功；
string sys_read_file(FCB* file,bool &flag);//读取文件内容；                //目前测试成功；
int sys_cd(string path, string &name);//进入子目录；			//目前测试成功；
void sys_cdback();//返回上一级目录；								//目前测试成功；
int sys_cut(string oldPath, int type, string newPath);//剪切；			//目前测试成功；
int sys_copy(string oldPath, int type, string newPath);//复制；         //目前测试，成功；
int sys_share(FCB* shared, FCB* sharing);
int sys_write_disk(string diskpath);
int sys_read_disk(string diskpath);

#endif //JIANG_LIBRARY_H