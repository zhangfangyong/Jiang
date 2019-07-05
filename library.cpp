#include <iostream>
#include <cstring>
#include<fstream>
#include "library.h"
//#include <stringstream>
//#include "FAT.h"

using namespace std;

#define _maxUsers 21
#define DirDepth 10            //瀛愮洰褰曚釜鏁帮紱
char* _virtualDiskAddr;   //铏氭嫙纾佺洏鐨勫叆鍙ｅ湴鍧€锛?
int _emptyBLOCK_Count;      //鍙敤鐨凚LOCK鏁伴噺锛?
//int _emptyFCB_Count;         //鍙敤鐨凢CB鐨勬暟閲忥紱
FCB* _current; //褰撳墠鐩綍锛?
string current_path;      //褰撳墠璺緞锛?
FAT* _fat;//绯荤粺FAT琛?
FCB* _root;   //鎸囧悜root锛屽疄鐜扮粷瀵硅矾寰勶紱
FCBBLOCK* _users;
FCB* current_user;
int current_usernum;
/**********************************************************************/
//纾佺洏绠＄悊澹版槑锛?
/**********************************************************************/
//鍒濆鍖栬櫄鎷熺鐩橈紱
//鐢变簬鐢ㄦ埛鍙兘闇€瑕佹仮澶嶄箣鍓嶇殑鍐呭瓨鐘舵€侊紝鎵€浠ヤ笉鎸夌収褰撲笅鐨勫垵濮嬪寲锛屾彁渚沬f鍒ゆ柇鍗冲彲銆?
void sys_initDisk() {
    _virtualDiskAddr = (char *)malloc(_virtualDiskSize * sizeof(char));   //澹拌绌洪棿锛歝har鏄竴涓瓧鑺傦紝鎵€浠ヨ櫄鎷熺鐩樼殑澶у皬鏄?M锛?

    _fat = new (_virtualDiskAddr)FAT();//鍒濆涓虹┖

    /*//寤虹珛鏍箁oot鍒濆鍖栨枃浠剁洰褰曪紱
    _root = new(_virtualDiskAddr + _FatBlockCount * _blockSize)FCB();
    strcpy(_root->_name, "root");
    _root->_parent = _root;
    _current = _root;
    current_path = "//";
    */
    ////_fat->fats[0]=-1!
    _fat->fats[1]=_systemUsedBlock;//鐢ㄦ埛鍧楋紒锛侊紒
    _users=new(_virtualDiskAddr + _systemUsedBlock * _blockSize)FCBBLOCK();
    _users->_contentLength=1;
    _users->_blockID = _systemUsedBlock;
    _root = &(_users->_content[0]);//????????
    current_user=_root;
    current_usernum=0;
    strcpy(_root->_name, "root");//type size榛樿
    ////_root->_block=;????????????????????????
    _current = _root;
    current_path = "//";
/*
    //鎶婄┖鐨凢CB閾炬帴璧锋潵锛?
    //char* _FCB_StartAllocation = _virtualDiskAddr + _bitMapCount * _blockSize + 64;    //64鏄痳oot鐨凢CB锛?
    _emptyFCB = new(_root + 1)FCB();
    _lastFCB = _emptyFCB;
    for (int i = 1; i < _FCB_count - 1; i++)   //杩樺墿_FCB_count-1涓狥CB锛屾瘡涓?4B锛?
    {
        _lastFCB->_child = new(_lastFCB + 1)FCB();
        _lastFCB = _lastFCB->_child;
    }
    _emptyFCB_Count = _FCB_count - 1;
*/
/*    //鍒濆鍖朆lock锛?
    BLOCK* initBlock;
    BLOCK* nextBlock;
    initBlock = new(_virtualDiskAddr + _systemUsedBlock * _blockSize)BLOCK();
    initBlock->_blockID = _systemUsedBlock;        //128;
    for (int i = 1; i < _BLOCK_count; i++)
    {
        nextBlock = new(initBlock + 1)BLOCK();
        initBlock=nextBlock;
        initBlock->_blockID = _systemUsedBlock + i;
    }
*/
///杩欓儴鍒嗚繕闇€瑕佸悧
/*
    //鎶婄┖鐨凚LOCK閾炬帴璧锋潵锛?
    _emptyBLOCK = new(_virtualDiskAddr + _systemUsedBlock * _blockSize)BLOCK();
    _emptyBLOCK->_blockID = _systemUsedBlock;        //128;
    _lastBLOCK = _emptyBLOCK;
    for (int i = 1; i < _BLOCK_count; i++)
    {
        _lastBLOCK->_nextBlock = new(_lastBLOCK + 1)BLOCK();
        _lastBLOCK = _lastBLOCK->_nextBlock;
        _lastBLOCK->_blockID = _systemUsedBlock + i;
    }
*/
    ////_emptyBLOCK_Count = _BLOCK_count;
    _emptyBLOCK_Count = _BLOCK_count-1;
}


//////////////////////////////////////////////////////////////////////////////////////////////
//鎶婂潡锛堣€冭檻鍧楅摼锛夊垵濮嬪寲涓篵lock
FileBLOCK* initFileBlock(int num) {
    FileBLOCK* head_block = new(_virtualDiskAddr + num*_blockSize)FileBLOCK();
    FileBLOCK* next_block ;
    int tmp=_fat->fats[num];
    while(tmp!=-1&&tmp!=128){
        next_block = new(_virtualDiskAddr + tmp*_blockSize)FileBLOCK();
        tmp=_fat->fats[tmp];
    }
    return head_block;
}
FCBBLOCK* initFCBBlock(int num) {//num涓€鑸负1
    FCBBLOCK* head_block = new(_virtualDiskAddr + num*_blockSize)FCBBLOCK();
    FCBBLOCK* next_block ;
    int tmp=_fat->fats[num];
    while(tmp!=-1&&tmp!=128){/////////////////////////////////////////128 重新new导致信息没了
        next_block = new(_virtualDiskAddr + tmp*_blockSize)FCBBLOCK();
        tmp=_fat->fats[tmp];
    }
    return head_block;
}
//鏍规嵁鍧楀彿鎵惧埌block锛屽浜庡凡缁忓垵濮嬪寲杩囩殑
FileBLOCK* num2FileBlock(int num) {
    FileBLOCK* B = (FileBLOCK*)(_virtualDiskAddr + num*_blockSize);
    return B;
}
FCBBLOCK* num2FCBBlock(int num) {
    FCBBLOCK* B = (FCBBLOCK*)(_virtualDiskAddr + num*_blockSize);
    return B;
}

//////////////////////////////////////////////////////////////////////////////////////////

//缁欏紑鍙戣€呮彁渚汢LOCK锛屼緵鍐欏叆鏂囦欢鏃朵娇鐢紱
/*
BLOCK* getEmptyBLOCKNum(int need_BLOCK_Num) {
	if (_BLOCK_count >= need_BLOCK_Num)
	{
		////block璁颁綇鍒嗛厤鐨勫ご
		BLOCK* block = _emptyBLOCK;
		for (int i = 0; i < need_BLOCK_Num - 1; i++)
		{
			_emptyBLOCK = _emptyBLOCK->_nextBlock;
		}
		////temp鏆傚瓨灏剧疆next绌轰粠鑰屼粠绌篵lock
		////empytyblock澶寸Щ鍔ㄥ埌鏂颁綅缃紝emptyblcokconut鍑忓皯
		BLOCK* tmp = _emptyBLOCK;
		_emptyBLOCK = _emptyBLOCK->_nextBlock;
		tmp->_nextBlock = NULL;
		_emptyBLOCK_Count -= need_BLOCK_Num;
		return block;
	}
	return NULL;
}*/
int getEmptyBLOCKNum(int need_BLOCK_Num) {//杩斿洖鍙娇鐢ㄧ┖鐧紹LOCK搴忓彿锛岃嚜鍔ㄨ鍏AT琛?
    if (_BLOCK_count >= need_BLOCK_Num)
    {
        ////block璁颁綇鍒嗛厤鐨勫ご
        int block;
        bool init=false;
        int tmp_block;
        int j = 129;//鍙紭鍖?
        for (int i = 0; i < need_BLOCK_Num; i++)
        {
            while (_fat->fats[j] != -1 ) j++;
            if (!init) {
                block = j;
                init = true;
                tmp_block = block;
            }
            else {
                _fat->fats[tmp_block] = j;
                tmp_block = j;
            }
            j++;
        }
        _fat->fats[tmp_block] = 128;
        _emptyBLOCK_Count -= need_BLOCK_Num;
        return block;
    }
    return -1;
}

//缁欏紑鍙戣€呮彁渚汧CB锛屼緵鍒涘缓鏂囦欢鎴栫洰褰曟椂浣跨敤锛?
/*
FCB* getFCB() {
    if (_FCB_count>0)
    {
        //璇存槑鏈夊彲鐢ㄧ殑FCB锛岄偅涔堝彧瑕佽繑鍥瀇emptyFCB涓殑涓€涓紝骞朵慨鏀笷CB杩欎釜閾捐〃锛?
        FCB* fcb = _emptyFCB;
        _emptyFCB = _emptyFCB->_child;
        fcb->_child = NULL;
        _emptyFCB_Count--;
        return fcb;
    }
    return NULL;         //鐢ㄦ埛璋冪敤鏂囦欢绯荤粺lib鏃讹紝闇€瑕佸垽鏂槸鍚︿负绌猴紝鑰屾彁渚涜緭鍑轰俊鎭紱
}
*/
FCB* getblankFCB(FCB* parentDir) {///////////////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
    //鎵惧埌destination鐨勭洰褰曟枃浠剁洏鍧楋紝鍙栧叾涓竴涓猣cb
    int tmp = parentDir->_block;
    FCBBLOCK *destination_fcbblock;
    int beforetmp;
    FCB *fcb;
    if(parentDir->_block!=-1) {
        destination_fcbblock = num2FCBBlock(parentDir->_block);
        while (destination_fcbblock->_contentLength == _fcbsSize) {
            beforetmp = tmp;
            tmp = _fat->fats[tmp];//鏌ユ壘涓嬩竴涓洏
            if (tmp == -1) {//鐗规畩鎯呭喌:姝ｅソ鎵€鏈夋弧鐩?
                //new涓€涓柊fcbblock锛屼笖璁板綍鍏at琛?
                tmp = getEmptyBLOCKNum(1);
                //destination_fcbblock=num2FCBBlock(tmp);
                destination_fcbblock = initFCBBlock(tmp);
                _fat->fats[beforetmp] = tmp;
                break;
            }
            destination_fcbblock = num2FCBBlock(tmp);
        }
        //鐩爣fcb
        fcb = &(destination_fcbblock->_content[destination_fcbblock->_contentLength]);////&&&&&&&!!!!!!

    }else{
        ///////////////////////////////////////////////////考虑还未有盘块的的目录！

        tmp = getEmptyBLOCKNum(1);

        destination_fcbblock = initFCBBlock(tmp);

        parentDir->_block = tmp;

        // _fat->fats[parentDir->_block] = 128;//????????FAT的修改不用
        fcb = &(destination_fcbblock->_content[destination_fcbblock->_contentLength]);
        ////////////////////////////////////////////////////////////
    }
    destination_fcbblock->_contentLength+=1;///!!!!
    fcb->_access[current_usernum]='4';////创建者最高其权限
    return fcb;
}


/*
//鍒犻櫎鏂囦欢鏃讹紝鍥炴敹骞舵竻鐞咮LOCK锛?
void releaseBLOCK(BLOCK* block) {
	if (block == NULL)
		return;
	_lastBLOCK->_nextBlock = block;
	_lastBLOCK = _lastBLOCK->_nextBlock;
	strcpy(_lastBLOCK->_content, "");
	while (_lastBLOCK != NULL)
	{
		_lastBLOCK->_contentLength = 0;
		_emptyBLOCK_Count++;
		_lastBLOCK = _lastBLOCK->_nextBlock;
	}
}
*/
//鍒犻櫎鏂囦欢鏃讹紝鍥炴敹骞舵竻鐞咮LOCK锛?////////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
void releaseBLOCK(int block,int type) {//閲婃斁鍐呭瓨绌洪棿锛屼慨鏀筬at琛?
    //閲婃斁棣栧潡
    if (block == -1)
        return;
    if(type==1){
        FCBBLOCK* rmfcbblock=num2FCBBlock(block);
        int num=0;
        while(num<rmfcbblock->_contentLength) {
            releaseBLOCK(rmfcbblock->_content[num]._block, rmfcbblock->_content[num]._type);
            num++;
        }
        delete rmfcbblock;
    }
    else{
        FileBLOCK* rmfileblock=num2FileBlock(block);
        delete rmfileblock;
    }
    _emptyBLOCK_Count++;
    //閲婃斁鍚庨潰鐨勫潡
    int next_block=_fat->fats[block];
    while (next_block != -1) {
        next_block = _fat->fats[block];
        _fat->fats[block] = -1;

        if(type==1){
            FCBBLOCK* rmfcbblock=num2FCBBlock(block);
            int num=0;
            while(num<rmfcbblock->_contentLength) {
                releaseBLOCK(rmfcbblock->_content[num]._block, rmfcbblock->_content[num]._type);
                num++;
            }
            delete rmfcbblock;
        }
        else{
            FileBLOCK* rmfileblock=num2FileBlock(block);
            delete rmfileblock;
        }
        _emptyBLOCK_Count++;
        block = next_block;
    }
}

//鍒犻櫎鏂囦欢/鐩綍鏃讹紝鍥炴敹骞舵竻鐞咶CB锛涖€併€併€併€併€併€佸垹闄ゅ甫鏉ュぇ楹荤儲,瀵瑰簲娣诲姞涔熻鏀?//淇敼瀹屾垚锛屾湭娴嬭瘯
void releaseFCB(FCBBLOCK* block,int fcbnum) {
    releaseBLOCK(block->_content[fcbnum]._block,block->_content[fcbnum]._type);
    //淇敼鏈琤lock
    while(fcbnum+1<(block->_contentLength)){
        block->_content[fcbnum]=block->_content[fcbnum+1];
        fcbnum++;
    }
    FCB* fcb=&(block->_content[fcbnum]);//&!!!
    strcpy(fcb->_name, "");
    fcb->_type = 1;
    fcb->_size = 0;
    fcb->_block = -1;
    //_emptyFCB_Count++;
    block->_contentLength--;
}

//鍒犻櫎鐩綍鍜屾枃浠朵腑鏈変釜鐩稿悓鐨勬搷浣滃氨鏄洖鏀禙CB锛岀幇鎶藉彇鍑烘潵锛?/////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
//鍒犻櫎鐩綍鍜屾枃s浠朵粠鍥炴敹FCB寮€濮?/////////////////////////////////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
void delete_dirOrFile(FCBBLOCK* block,int fcbnum) {
    releaseFCB(block,fcbnum);
    //鍥炴敹FCB锛?
    /*
   if (deleting->_type == 1)
   {
       //鍒犻櫎鐨勬槸鐩綍锛?
       //鍏堝垹闄よ繖涓洰褰曚腑鐨勫唴瀹癸紱
       //鍒犻櫎杩欎釜鐩綍涓椂锛岄渶瑕佸厛鍒犻櫎child////brother锛?
       while (deleting->_child != NULL)
       {
           delete_dirOrFile(deleting->_child);
       }
       recoverFCB(deleting);
   }
   else
   {
       //鏄枃浠讹紱鐩存帴鍥炴敹灏卞ソ锛?
       recoverFCB(deleting);
   }*/
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
//鍦ㄥ綋鍓嶇洰褰曚笅鎼滅储FCB锛?
FCB* returnSonFCB(FCB* currentDir, string name,int type,int &B,int &F) {
    int fcbnum;
    int tmp_blocknum=currentDir->_block;
    FCBBLOCK* tmp_fcbblock;
    FCB* fcb;

    while(tmp_blocknum!=-1){

        tmp_fcbblock=num2FCBBlock(tmp_blocknum);
        fcbnum=0;
        while (fcbnum<(tmp_fcbblock->_contentLength))
        {
            fcb=&(tmp_fcbblock->_content[fcbnum]);
            if (fcb->_name == name && fcb->_type == type)
            {

                B=tmp_blocknum;
                F=fcbnum;
                //璇存槑鎵惧埌浜嗭紱
                return fcb;
            }
            fcbnum++;
        }
        tmp_blocknum=_fat->fats[tmp_blocknum];
    }
    return NULL;           //鐢卞紑鍙戣€呮嵁姝ゆ彁渚涢敊璇俊鎭紱
}
/*
//杩斿洖parent锛屽湪缁欏畾鐨勭洰褰曚腑锛屽垽鏂洰褰曟槸鍚﹀瓨鍦紝涓嶅瓨鍦ㄨ繑鍥濶ULL锛屽瓨鍦ㄨ繑鍥炶繖涓洰褰旻CB锛?
FCB* return_DIR_FCB(string DIR[], int count, bool isAbsolutePath)
{
    FCB* tmp;
    if (isAbsolutePath)
    {
        tmp = _root;
    }
    else
    {
        tmp = _current;
    }

    for (int i = 0; i < count; i++)
    {
        tmp = returnSonFCB(tmp, DIR[i], 1);
        if (tmp == NULL)
        {
            return NULL;           //璇存槑褰撳墠鐩綍涓嬫病鏈夎繖涓瓙鐩綍锛?
        }
    }
    return tmp;
}*/
FCB* return_FCB(string DIR[], int count, int type,FCB* &parent,int &B,int &F,bool isAbsolutePath){
    FCB* currentDir;
    currentDir=_current;
    if(isAbsolutePath){
        int tmp_c=0;
        while(tmp_c<count-1) {
            currentDir=returnSonFCB(currentDir,DIR[tmp_c],1,B,F);
            if(currentDir==NULL) return NULL;
            tmp_c++;
        }
    }
    else currentDir=_current;
    parent =currentDir;//鐖剁洰褰曞瓨鍦?
    return returnSonFCB(currentDir,DIR[count-1],type,B,F);
}


////鍙互瀹炵幇璺緞---瀵圭洰褰曞拰鏂囦欢鐨勮闂紱//姝ゅ鏈夌枒闂紵锛燂紵锛燂紵锛燂紵锛燂紵
/////鎬荤殑姒傛嫭鐨勶紝灞傜骇璋冪敤鍓嶉潰鐨?
FCB* sys_returnFCB(string path_name, string &name, int type,FCB* &parent,int &B,int &F) {//鏀寔缁濆璺緞鍜岀浉瀵硅矾寰?--鎵惧埌鎵€闇€FCB
    string* DIR = new string[DirDepth]; //string DIR[DirDepth];
    int i = 0;
    ////鎷嗗垎璺緞娈?
    for (int j = 0; j < path_name.length(); j++) {
        if (path_name[j] == '/') {
            if (DIR[i] != "")
                i++;
            else continue;
        }
        else {
            DIR[i] += path_name[j];
        }
    }
    ////缁熻璺緞娈典釜鏁?
    int count = 0;
    for (i = 0; i < 20; i++) {
        if (DIR[i] != "") {
            count++;
        }
        else {
            break;
        }
    }
    if (count == 0&&path_name[0]=='/')//涓や釜鏉′欢鍚屾椂婊¤冻涓簉oot
    {
        //璇存槑鐢ㄦ埛鐨勮矾寰?
        name = "root";
        return _root;
    }
    else
    {
        name = DIR[count - 1];      //杩欎釜涓鸿矾寰勪腑鐨勬渶鍚庝竴涓瓧娈碉紱
    }

    bool isAbsolutePath=false;
    if (path_name[0] == '/') isAbsolutePath=true;

    return return_FCB(DIR, count,type,parent,B,F,isAbsolutePath);

/*
    if (count == 0&&path_name[0]=='/')//涓や釜鏉′欢鍚屾椂婊¤冻涓簉oot
    {
        //璇存槑鐢ㄦ埛鐨勮矾寰?
        name = "root";
        parent = _root;
        return _root;
    }
    else
    {
        name = DIR[count - 1];      //杩欎釜涓鸿矾寰勪腑鐨勬渶鍚庝竴涓瓧娈碉紱
    }

    bool AbsolutePath = false;
    if (path_name[0] == '/') {
        AbsolutePath = true;
    }
    parent = return_DIR_FCB(DIR, count - 1, AbsolutePath);
    if (parent != NULL)
    {

        return returnSonFCB(parent, name, type);
    }
    else
    {
        return NULL;
    }
	*/
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//姹傝澶嶅埗鐨勶紝闇€瑕佸灏戠郴缁熻祫婧愶紝FCB涓暟锛孊LOCK涓暟//////////////////////////鍘熶唬鐮佸凡琚慨鏀?瀹屾垚锛屾湭娴嬭瘯
int num_dirOrFile(FCB* adding) {

    //numFCB++;
    int numBLOCK=0;
    int type = adding->_type;
    if (type == 0)//璇存槑adding鏄釜鏂囦欢锛?
    {
        //鍙渶瑕佽绠梐dding鐨凚LOCK锛?
/*这个方法存在bug
        double needAddBLOCK;
        needAddBLOCK = adding->_size / _contentSize;
        int tmp = needAddBLOCK;
*/
        int needAddBLOCK;
        needAddBLOCK = adding->_size / _contentSize;
        int tmp = adding->_size % _contentSize;
        if (tmp!=0)
        {
            //璇存槑娌℃崯澶憋紝鎵€浠ュ垎閰嶇殑涓暟涓簍mp;
            numBLOCK += 1;
        }
    }
    else//璇存槑adding鏄釜鐩綍锛?
    {//鎵惧埌adding涓嬬殑鎵€鏈夊唴瀹?
        //FCB* tmp=adding->child;
        FCBBLOCK* tmpblock=num2FCBBlock(adding->_block);
        int i=0;
        while (i<tmpblock->_contentLength)
        {
            //閫掑綊璁＄畻鐩綍涓嬪唴瀹癸紱
            numBLOCK+=num_dirOrFile(&(tmpblock->_content[i]));
            //tmp = tmp->_brother;
            i++;
        }
        int tmp=_fat->fats[adding->_block];
        while(tmp!=-1){
            tmpblock=num2FCBBlock(tmp);
            i=0;
            while (i<tmpblock->_contentLength)
            {
                //閫掑綊璁＄畻鐩綍涓嬪唴瀹癸紱
                numBLOCK+=num_dirOrFile(&(tmpblock->_content[i]));
                i++;
            }
            tmp=_fat->fats[tmp];
        }
    }
    return numBLOCK;
}

//澶嶅埗,褰撴槸鐩綍鏃讹紝闇€瑕乧opy鐩綍閲岀殑鎵€鏈夊唴瀹癸紱/////////////////////////////// /鍘熶唬鐮佸凡琚慨鏀?淇敼瀹屾垚锛屾湭娴嬭瘯
/*
void add_dirOrFile(FCB* adding, FCB* destination) {

    //destination 锛屽鍒跺埌鐨勯偅涓洰褰曪紱adding锛岄渶瑕佸鍒剁殑鍐呭锛?
    //鍥犱负瑕佸鍔犺繖涓枃浠?鐩綍锛屾墍浠ラ兘闇€瑕佹妸杩欎釜鑺傜偣澶嶅埗涓€浠界Щ鍏ョ洰褰曟爲涓紱
    FCB* copy_adding = getFCB();
    strcpy(copy_adding->_name, adding->_name);
    //copy_adding->_spare = adding->_spare;
    copy_adding->_type = adding->_type;
    copy_adding->_size = adding->_size;
    copy_adding->_block = adding->_block;
    //杩欐槸鍖哄垎鐩綍鍜屾枃浠剁殑鍏抽敭锛?
    copy_adding->_child = NULL;
    ////涓嬮潰涓よ瀹炵幇浜嗙Щ鍏ョ洰褰曟爲涓紱
    copy_adding->_parent = destination;
    copy_adding->_brother = destination->_child;
    destination->_child = copy_adding;

    //杩欓噷蹇呴』鏂帀brother;
    //copy_adding->_child->_brother = NULL;


    FCB* tmp;//淇濆瓨绉诲姩鐩爣鐨勫瓙鑺傜偣---鏂囦欢or涓嬩竴绾?
    tmp = adding->_child;

    if (tmp == NULL)
    {
        //璇存槑adding鏄釜鏂囦欢锛?
        //鍒欓渶瑕乤dd澹拌BLOCK锛屽瓨鏀惧唴瀹癸紱

        //璇ユ枃浠舵湁鍐呭锛岄渶瑕佸娣籅LOCK锛?

        //闇€瑕佹坊鍔犲嚑涓紱
        double needAddBLOCK;

        needAddBLOCK = copy_adding->_size / _contentSize;
        if (needAddBLOCK==0)
        {
            //涓嶉渶瑕丅LOCK锛?
        }
        else
        {
            int tmp = needAddBLOCK;

            ////BLOCK* contentFirstAdrr;   //鍒嗛厤鐨凚LOCK鐨勯鍦板潃锛?
            int contentFirstBlockNum;

            if (needAddBLOCK == tmp)
            {
                //璇存槑娌℃崯澶憋紝鎵€浠ュ垎閰嶇殑涓暟涓簍mp;
                ////contentFirstAdrr = getEmptyBLOCKNum(tmp);
                contentFirstBlockNum = getEmptyBLOCKNum(tmp);

            }
            else
            {
                //绮惧害鎹熷け锛涗笂鍙栨暣锛?
                ////contentFirstAdrr = getEmptyBLOCKNum(tmp + 1);
                contentFirstBlockNum = getEmptyBLOCKNum(tmp);
            }

            ////copy_adding->_block = contentFirstAdrr;
            copy_adding->_block = contentFirstBlockNum;

            //鎸囧悜adding鐨凚LOCK锛?
            ////BLOCK* tmp_addingBLOCK;
            ////tmp_addingBLOCK = adding->_block;
            int tmp_addingBLOCKNum = adding->_block;
            BLOCK* contentFirstAdrr = num2Block(contentFirstBlockNum);
            BLOCK* tmp_addingBLOCK = num2Block(tmp_addingBLOCKNum);
            //鍏堝鐞嗘暣鏁板唴瀹圭殑BLOCK锛?
            for (int i = 0; i < tmp - 1; i++)
            {
                ////strcpy(contentFirstAdrr->_content, ((string)(tmp_addingBLOCK->_content)).substr(i*_contentSize, _contentSize).c_str());
                strcpy(contentFirstAdrr->_content, ((string)(tmp_addingBLOCK->_content)).substr(0, _contentSize).c_str());
                contentFirstAdrr->_contentLength = _contentSize;
                contentFirstAdrr = num2Block(_fat->fats[contentFirstBlockNum]);
                contentFirstBlockNum = _fat->fats[contentFirstBlockNum];//-1??????
                tmp_addingBLOCK = num2Block(_fat->fats[contentFirstBlockNum]);
                tmp_addingBLOCKNum = _fat->fats[tmp_addingBLOCKNum];//-1??????
            }
            //瀛樺彇鏈€鍚庝竴涓墿浣欏唴瀹癸紱
            contentFirstAdrr->_contentLength = tmp_addingBLOCK->_contentLength;
            strcpy(contentFirstAdrr->_content, ((string)(tmp_addingBLOCK->_content)).substr(0, contentFirstAdrr->_contentLength).c_str());
            /*
            //鍏堝鐞嗘暣鏁板唴瀹圭殑BLOCK锛?
            for (int i = 0; i < tmp - 1; i++)
            {//////锛燂紵锛燂紵锛燂紵锛燂紵锛燂紵锛燂紵锛燂紵锛燂紵锛?
                strcpy(contentFirstAdrr->_content, ((string)(tmp_addingBLOCK->_content)).substr(i*_contentSize, _contentSize).c_str());
                contentFirstAdrr->_contentLength = _contentSize;
                contentFirstAdrr = contentFirstAdrr->_nextBlock;
                tmp_addingBLOCK = tmp_addingBLOCK->_nextBlock;
            }
            //瀛樺彇鏈€鍚庝竴涓墿浣欏唴瀹癸紱
            contentFirstAdrr->_contentLength = tmp_addingBLOCK->_contentLength;
            strcpy(contentFirstAdrr->_content, ((string)(tmp_addingBLOCK->_content)).substr(0, contentFirstAdrr->_contentLength).c_str());
            //
        }
    }
    else
    {
        //鍏堟嫹璐漜hild;
        add_dirOrFile(tmp, copy_adding);
        tmp = tmp->_brother;
        while (tmp != NULL)
        {
            //鏈川鏄嫹璐濇枃浠讹紝鎵€浠ユ枃浠?鐩綍瀹炵幇鏂瑰紡鐩稿悓锛涢€掑綊璋冪敤--娉ㄦ剰閫掑綊鍑哄彛涔熷嵆缁熶竴鎬?
            add_dirOrFile(tmp, copy_adding);
            tmp = tmp->_brother;
        }
    }

}*/

void add_dirOrFile(FCB* adding, FCB* destination) {//destination 锛屽鍒跺埌鐨勯偅涓洰褰曪紱adding锛岄渶瑕佸鍒剁殑鍐呭锛?/鍗曠函鎵ц澶嶅埗鎿嶄綔锛屼笉鐢ㄨ€冭檻閲嶅悕绛夐棶棰?
/*
	//鎵惧埌destination鐨勭洰褰曟枃浠剁洏鍧楋紝鍙栧叾涓竴涓猣cb
	FCBBLOCK* destination_fcbblock=num2FCBBlock(destination->_block);
	int tmp=destination->_block;
	int beforetmp;
	while(destination_fcbblock->_contentLength==_fcbsSize){
		beforetmp=tmp;
		tmp=_fat->fats[tmp];//鏌ユ壘涓嬩竴涓洏
		if(tmp==-1){//鐗规畩鎯呭喌:姝ｅソ鎵€鏈夋弧鐩?
			//new涓€涓柊fcbblock锛屼笖璁板綍鍏at琛?
			tmp=getEmptyBLOCKNum(1);
			//destination_fcbblock=num2FCBBlock(tmp);
			destination_fcbblock=initFCBBlock(tmp);
			_fat->fats[beforetmp]=tmp;
			break;
		}
		destination_fcbblock=num2FCBBlock(tmp);
	}
	//璁剧疆鐩爣fcb
	FCB* destination_fcb=destination_fcbblock->_content[destination_fcbblock->_contentLength];*/
    FCB* destination_fcb=getblankFCB(destination);
    //鏍规嵁addingfcb璁剧疆鐩爣fcb鍩烘湰淇℃伅
    strcpy(destination_fcb->_name, adding->_name);
    destination_fcb->_type = adding->_type;
    destination_fcb->_size = adding->_size;
    ////copy_adding->_block = adding->_block;



    int needAddBLOCK=num_dirOrFile(adding);
    int contentFirstBlockNum = getEmptyBLOCKNum(needAddBLOCK);

    //璁剧疆FCB鏁版嵁缁撴瀯涓墿鐞嗗瓨鍌˙LOCK//鐩綍鏄疐CBBLOCK锛屾枃浠舵槸FileBLOCK
    int type = adding->_type;
    if (type == 0)
    {
        //璇存槑adding鏄釜鏂囦欢锛?
        //鍒欓渶瑕乤dd澹拌BLOCK锛屽瓨鏀惧唴瀹癸紱

        //璇ユ枃浠舵湁鍐呭锛岄渶瑕佸娣籅LOCK锛?
        //闇€瑕佹坊鍔犲嚑涓紱
        /*
        double needAddBLOCK;
        needAddBLOCK = adding->_size / _contentSize;
        if (needAddBLOCK==0)
        {
            //涓嶉渶瑕丅LOCK锛?
        }
        else
        {
            int tmp = needAddBLOCK;

            ////BLOCK* contentFirstAdrr;   //鍒嗛厤鐨凚LOCK鐨勯鍦板潃锛?
            int contentFirstBlockNum;

            if (needAddBLOCK == tmp)
            {
                //璇存槑娌℃崯澶憋紝鎵€浠ュ垎閰嶇殑涓暟涓簍mp;
                ////contentFirstAdrr = getEmptyBLOCKNum(tmp);
                contentFirstBlockNum = getEmptyBLOCKNum(tmp);

            }
            else
            {
                //绮惧害鎹熷け锛涗笂鍙栨暣锛?
                ////contentFirstAdrr = getEmptyBLOCKNum(tmp + 1);
                contentFirstBlockNum = getEmptyBLOCKNum(tmp+1);
            }
            */
        destination_fcb->_block = contentFirstBlockNum;

        FileBLOCK* contentFirstAdrr =initFileBlock(contentFirstBlockNum);//num2FileBlock(contentFirstBlockNum);

        //鎸囧悜adding鐨凚LOCK锛?
        ////BLOCK* tmp_addingBLOCK;
        ////tmp_addingBLOCK = adding->_block;
        int tmp_addingBLOCKNum = adding->_block;
        FileBLOCK* tmp_addingBLOCK = num2FileBlock(tmp_addingBLOCKNum);
        //鍏堝鐞嗘暣鏁板唴瀹圭殑BLOCK锛?
        for (int i = 0; i < needAddBLOCK - 1; i++)
        {
            ////strcpy(contentFirstAdrr->_content, ((string)(tmp_addingBLOCK->_content)).substr(i*_contentSize, _contentSize).c_str());
            strcpy(contentFirstAdrr->_content, ((string)(tmp_addingBLOCK->_content)).substr(0, _contentSize).c_str());
            contentFirstAdrr->_contentLength = _contentSize;
            contentFirstAdrr = num2FileBlock(_fat->fats[contentFirstBlockNum]);
            contentFirstBlockNum = _fat->fats[contentFirstBlockNum];//-1??????
            tmp_addingBLOCK = num2FileBlock(_fat->fats[contentFirstBlockNum]);
            tmp_addingBLOCKNum = _fat->fats[tmp_addingBLOCKNum];//-1??????
        }
        //瀛樺彇鏈€鍚庝竴涓墿浣欏唴瀹癸紱
        contentFirstAdrr->_contentLength = tmp_addingBLOCK->_contentLength;
        strcpy(contentFirstAdrr->_content, ((string)(tmp_addingBLOCK->_content)).substr(0, contentFirstAdrr->_contentLength).c_str());
    }

    else//adding鏄洰褰曟枃浠?
    {

        //double needAddBLOCK;
        //needAddBLOCK = adding->_size / _fcbsSize;
        if(needAddBLOCK==0){
        }else{
            int contentFirstBlockNum;
            contentFirstBlockNum = getEmptyBLOCKNum(needAddBLOCK);
            destination->_block = contentFirstBlockNum;
            FCBBLOCK* contentFirstAdrr =initFCBBlock(contentFirstBlockNum);

            int childfcb_num;
            int childblock_num=contentFirstBlockNum;
            FCBBLOCK* childFCBBlock;
            FCB* childfcb;
            while(childblock_num!=-1){
                childFCBBlock=num2FCBBlock(childblock_num);
                childfcb_num=0;
                while(childfcb_num<childFCBBlock->_contentLength){
                    childfcb=&(childFCBBlock->_content[childfcb_num]);
                    add_dirOrFile(childfcb, destination_fcb);
                    childfcb_num++;
                }
                childblock_num=_fat->fats[childblock_num];
            }
        }

    }

}

int share_dirOrFile(FCB* shared, FCB* sharing) {
    if(shared->_access[current_usernum]!='0') {
        sharing->_block = shared->_block;
        return 1;
    }else return 0;
}


/**********************************************************************/
//鐩綍绠＄悊澹版槑锛?
/**********************************************************************/
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int sys_createuser(string username){
    if(_users->_contentLength<_fcbsSize){
        int u=0;
        while(u<_users->_contentLength){//鐢ㄦ埛涓嶅瓨鍦?
            if(_users->_content[u]._name==username) ///???????璇硶鏄惁姝ｇ‘
                return -2;//鐢ㄦ埛宸插瓨鍦?
            u++;
        }
        FCB* user=&(_users->_content[u]);
        strcpy(user->_name, username.c_str());
        user->_access[u]='4';
        _users->_contentLength+=1;

        return 1;//鐢ㄦ埛娣诲姞鎴愬姛
    }
    else return -1;//娣诲姞鐢ㄦ埛澶辫触锛岀敤鎴疯揪涓婇檺
}

int sys_suroot(){
    _current=_root;
    current_path="//";
    current_usernum=0;
    current_user=_root;
    return 1;//鐢ㄦ埛宸插瓨鍦?

}
int sys_su(string name){
    int u=0;
    while(u<_users->_contentLength){//鐢ㄦ埛涓嶅瓨鍦?
        if(_users->_content[u]._name==name){ ///???????璇硶鏄惁姝ｇ‘
            _current=&(_users->_content[u]);
            current_path="//";
            current_usernum=u;
            current_user=&(_users->_content[u]);
            return 1;//鐢ㄦ埛宸插瓨鍦?
        }
        u++;
    }
    return -1;//鐢ㄦ埛娣诲姞鎴愬姛
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//鏌ョ湅褰撳墠鐩綍鍐呭锛?/stringlist杩斿洖鎵撳嵃鍐呭(姝ｇ‘鍐呭or鐩綍涓嶅瓨鍦?
//鏌ョ湅褰撳墠鐩綍鍐呭锛涙墿灞曪細瀹炵幇鏌ョ湅浠绘剰璺緞涓嬬殑鍐呭锛?///////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
StringList* sys_dir(string dirPath) {

    string dirName = "";             //鐢ㄦ埛鎯宠鏌ョ湅鐨勭洰褰曠殑鍚嶅瓧锛涘湪涓嬮潰鍑芥暟鑾峰緱锛?
    const int dir_type = 1;            //涓€瀹氭槸鐩綍锛?
    //闇€瑕佺煡閬撶敤鎴锋兂鏌ョ湅鐨勭洰褰曞瓨涓嶅瓨鍦紱
    FCB*  parentDir = NULL;      //parentDir鐖剁洰褰曪紝鐩墠鏄笉鐭ラ亾鐨勶紝鎵€浠ヨ缃负NULL锛岄€氳繃涓嬭堪杩囩▼锛屽彲浠ョ煡閬損arentDir鏄惁瀛樺湪锛屼互鍙婃兂鏌ョ湅鐨勭洰褰曟槸鍚﹀瓨鍦紱
    int B,F;
    StringList *dir_content = new StringList();//鐢⊿tringList 瀛樺彇闇€瑕佹墦鍗扮殑淇℃伅锛?
    StringList *tmp_dirContent;
    FCB* tmp;

    if (dirPath != ""){
        ////////////////////////////////////////////////////
        if (dirPath[0] != '\\') {
            dir_content->content = "路径格式错误！\n";
            dir_content->next = NULL;
            return dir_content;
        }
        ////////////////////////////////////////////////////
        tmp = sys_returnFCB(dirPath, dirName, dir_type, parentDir,B,F);       //鑾峰緱瑕佹煡鐪嬬殑鐩綍锛?
    }
    else tmp = _current;

    if (tmp != NULL) {//瑕佹煡鐪嬬殑鐩綍瀛樺湪锛?
        if (tmp->_access[current_usernum] == '0') {
            dir_content->content += "当前目录您无权访问\n";
            dir_content->next = NULL;
            return dir_content;
        }else {
            tmp_dirContent = dir_content;//tmp_dirContent锛宒ir_content鐨勪复鏃跺壇鏈紝鐢变簬鏄寚閽堝唴瀹瑰啓鍏ョ殑鏄悓涓€鍦板潃
            if (tmp->_block == -1) {
                dir_content->content += "当前目录为空\n";
                dir_content->next = NULL;
                return dir_content;
            } else {
                int fcbblocknum = tmp->_block;
                FCBBLOCK *fcbblock = num2FCBBlock(fcbblocknum);
                if (fcbblock->_contentLength == 0) {
                    dir_content->content += "当前目录为空\n";
                    dir_content->next = NULL;
                    return dir_content;
                }
                int fcbnum;
                FCB *fcb;
                while (fcbblocknum != -1&&fcbblocknum != 128) {
                    fcbnum = 0;
                    fcbblock = num2FCBBlock(fcbblocknum);
                    while (fcbnum < fcbblock->_contentLength) {
                        fcb = &(fcbblock->_content[fcbnum]);
                        if (fcb->_type == 1) {
                            //cout << "Dir:	" << tmp->name << endl;
                            tmp_dirContent->content +=
                                    "Dir:" + (string) fcb->_name + " size:" + to_string(fcb->_size) + "\n";
                            tmp_dirContent->next = new StringList();
                            tmp_dirContent = tmp_dirContent->next;
                        } else {
                            tmp_dirContent->content +=
                                    "File:" + (string) fcb->_name + " size:" + to_string(fcb->_size) + "\n";
                            tmp_dirContent->next = new StringList();
                            tmp_dirContent = tmp_dirContent->next;
                        }
                        fcbnum++;
                    }
                    fcbblocknum = _fat->fats[fcbblocknum];
                }
            }
            return dir_content;
        }
    }
    else
    {
        if (parentDir==NULL)
        {
            dir_content->content = "该目录的父目录不存在！\n";
            dir_content->next = NULL;
            return dir_content;
        }
        else
        {
            //璇存槑鎯宠鍒犻櫎鐨勭洰褰曚笉瀛樺湪锛?
            dir_content->content = "该目录不存在！\n";
            dir_content->next = NULL;
            return dir_content;
        }
    }
}

//鍒涘缓鐩綍锛?//////////////////////////////////////////////////(fat璁╀唬鐮佹洿绠€鍗曚簡)淇敼瀹屾垚锛屾湭娴嬭瘯
int sys_mkdir(string dirPath) {

    const int dir_type = 1;

    //闇€瑕佺煡閬撶敤鎴锋兂鍒涘缓鐨勭洰褰曞瓨涓嶅瓨鍦紱
    FCB*  parentDir = NULL;      //parentDir鐖剁洰褰曪紝鐩墠鏄笉鐭ラ亾鐨勶紝鎵€浠ヨ缃负NULL锛岄€氳繃涓嬭堪杩囩▼锛屽彲浠ョ煡閬損arentDir鏄惁瀛樺湪锛屼互鍙婃兂鍒涘缓鐨勭洰褰曟槸鍚﹀瓨鍦紱
    string dirName = "";             //鐢ㄦ埛鎯宠鍒涘缓鐨勭洰褰曠殑鍚嶅瓧锛涘湪涓嬮潰鍑芥暟鑾峰緱锛?
    int B,F;
    if (sys_returnFCB(dirPath, dirName, dir_type, parentDir,B,F) == NULL) {
        //璇存槑鎯宠鍒涘缓鐨勭洰褰曚笉瀛樺湪锛岀户缁垽鏂兂鍦ㄥ摢涓洰褰曞垱寤猴紝閭ｄ釜鐩綍鏄惁瀛樺湪

        if (parentDir!=NULL)
        {
            if (parentDir->_access[current_usernum] == '3' || parentDir->_access[current_usernum] == '4') {

                FCB *dirfcb = getblankFCB(parentDir);
                strcpy(dirfcb->_name, dirName.c_str());
                //鐩綍鍒涘缓鎴愬姛锛涜繑鍥?锛涙彁绀哄紑鍙戣€咃紝鍚戠敤鎴锋樉绀烘垚鍔熶俊鎭紱
                return 1;
            }else return 0;
        }
        else
        {
            //鎻愮ず寮€鍙戣€咃紝鐢卞紑鍙戣€呭憡鐭ョ敤鎴凤紝鎯宠鍒涘缓鐨勯偅涓洰褰曟墍鍦ㄧ殑鐩綍涓嶅瓨鍦紝搴旇鍏堝垱寤哄畠鐨勭埗鐩綍锛?
            return -1;
        }
    }
    else
    {
        //鎻愮ず寮€鍙戣€咃紝鐢卞紑鍙戣€呭憡鐭ョ敤鎴凤紝鎯宠鍒涘缓鐨勭洰褰曞凡瀛樺湪锛屼笉鍏佽鍒涘缓锛?
        return -2;
    }
}

//鍒犻櫎鐩綍;/////////////////////////////////////////////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
int sys_rmdir(string dirPath) {
    const int dir_type = 1;

    //闇€瑕佺煡閬撶敤鎴锋兂鍒犻櫎鐨勭洰褰曞瓨涓嶅瓨鍦紱
    FCB*  parentDir = NULL;      //parentDir鐖剁洰褰曪紝鐩墠鏄笉鐭ラ亾鐨勶紝鎵€浠ヨ缃负NULL锛岄€氳繃涓嬭堪杩囩▼锛屽彲浠ョ煡閬損arentDir鏄惁瀛樺湪锛屼互鍙婃兂鍒犻櫎鐨勭洰褰曟槸鍚﹀瓨鍦紱
    string dirName = "";             //鐢ㄦ埛鎯宠鍒犻櫎鐨勭洰褰曠殑鍚嶅瓧锛涘湪涓嬮潰鍑芥暟鑾峰緱锛?
    int B,F;
    FCB* tmp_deleteDir; //鎸囧悜瑕佸垹闄ょ殑鐩綍锛?
    tmp_deleteDir = sys_returnFCB(dirPath, dirName, dir_type, parentDir,B,F);

    if (tmp_deleteDir != NULL) {
        //璇存槑鎯宠鍒犻櫎鐨勭洰褰曞瓨鍦紱
        if (tmp_deleteDir->_access[current_usernum] == '3' || tmp_deleteDir->_access[current_usernum] == '4') {

            delete_dirOrFile(num2FCBBlock(B), F);
            return 1;                          //鎻愮ず寮€鍙戣€咃紝鐢卞紑鍙戣€呭憡鐭ョ敤鎴凤紝鍒犻櫎鐩綍鎴愬姛锛?
        }else return 0;
    }
    else
    {
        if (parentDir == NULL)
        {
            return -1;          //鎻愮ず寮€鍙戣€咃紝鎯宠鍒犻櫎鐨勭洰褰曠殑鐖剁洰褰曚笉瀛樺湪锛?
        }
        else
        {
            //璇存槑鎯宠鍒犻櫎鐨勭洰褰曚笉瀛樺湪锛?
            return -2;               //鎻愮ず寮€鍙戣€咃紝鐢卞紑鍙戣€呭憡鐭ョ敤鎴凤紝鍒犻櫎鐨勭洰褰曚笉瀛樺湪锛?
        }
    }

}
/*
*鏂囦欢鎿嶄綔
*/
//鍒涘缓鏂囦欢锛?////////////////////////////////////////////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
int sys_create_file(string filePath) {
    const int file_type = 0;//闃叉璇敼

    //闇€瑕佺煡閬撶敤鎴锋兂鍒涘缓鐨勬枃浠跺瓨涓嶅瓨鍦紱
    FCB*  parentDir = NULL;      //parentDir鐖剁洰褰曪紝鐩墠鏄笉鐭ラ亾鐨勶紝鎵€浠ヨ缃负NULL锛岄€氳繃涓嬭堪杩囩▼锛屽彲浠ョ煡閬損arentDir鏄惁瀛樺湪锛屼互鍙婃兂鍒涘缓鐨勬枃浠舵槸鍚﹀瓨鍦紱
    string fileName = "";             //鐢ㄦ埛鎯宠鍒涘缓鐨勬枃浠剁殑鍚嶅瓧锛涘湪涓嬮潰鍑芥暟鑾峰緱锛?
    int B,F;
    if (sys_returnFCB(filePath, fileName, file_type, parentDir,B,F) == NULL) {
        //璇存槑鎯宠鍒涘缓鐨勬枃浠朵笉瀛樺湪锛岀户缁垽鏂兂鍦ㄥ摢涓洰褰曞垱寤猴紝閭ｄ釜鐩綍鏄惁瀛樺湪
        if (parentDir != NULL)
        {
            if (parentDir->_access[current_usernum] == '3' || parentDir->_access[current_usernum] == '4') {
                //璇存槑鎯冲湪閭ｄ釜鐩綍鍒涘缓鏄悎娉曠殑锛?
                //鍐呮牳鍒嗛厤FCB锛?
                FCB *fcboffile;
                fcboffile = getblankFCB(parentDir);     //杩斿洖缁欏紑鍙戣€呬竴涓狥CB锛涘緟寮€鍙戣€呬娇鐢紱
                strcpy(fcboffile->_name, fileName.c_str());
                fcboffile->_type = 0;
                fcboffile->_block = -1;
                fcboffile->_size = 0;
                //鏂囦欢鍒涘缓鎴愬姛锛涜繑鍥?锛涙彁绀哄紑鍙戣€咃紝鍚戠敤鎴锋樉绀烘垚鍔熶俊鎭紱
                return 1;
            }else return 0;

        }
        else
        {
            //鎻愮ず寮€鍙戣€咃紝鐢卞紑鍙戣€呭憡鐭ョ敤鎴凤紝鎯宠鍒涘缓鐨勯偅涓枃浠舵墍鍦ㄧ殑鐩綍涓嶅瓨鍦紝搴旇鍏堝垱寤哄畠鐨勭埗鐩綍锛?
            return -1;
        }
    }
    else
    {
        //鎻愮ず寮€鍙戣€咃紝鐢卞紑鍙戣€呭憡鐭ョ敤鎴凤紝鎯宠鍒涘缓鐨勬枃浠跺凡瀛樺湪锛屼笉鍏佽鍒涘缓锛?
        return -2;
    }
}

//鍒犻櫎鏂囦欢锛?///////////////////////////////////////////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
int sys_delete_file(string filePath) {

    const int file_type = 0;

    //闇€瑕佺煡閬撶敤鎴锋兂鍒犻櫎鐨勬枃浠跺瓨涓嶅瓨鍦紱
    FCB*  parentDir = NULL;      //parentDir鐖剁洰褰曪紝鐩墠鏄笉鐭ラ亾鐨勶紝鎵€浠ヨ缃负NULL锛岄€氳繃涓嬭堪杩囩▼锛屽彲浠ョ煡閬損arentDir鏄惁瀛樺湪锛屼互鍙婃兂鍒犻櫎鐨勬枃浠舵槸鍚﹀瓨鍦紱
    string fileName = "";             //鐢ㄦ埛鎯宠鍒犻櫎鐨勬枃浠剁殑鍚嶅瓧锛涘湪涓嬮潰鍑芥暟鑾峰緱锛?
    int B,F;
    FCB* tmp_deleteFile; //鎸囧悜瑕佸垹闄ょ殑鏂囦欢鎺у埗鍧楋紱
    tmp_deleteFile = sys_returnFCB(filePath, fileName, file_type, parentDir,B,F);

    if (tmp_deleteFile != NULL) {
        if (tmp_deleteFile->_access[current_usernum] == '3' || tmp_deleteFile->_access[current_usernum] == '4') {
            //璇存槑鎯宠鍒犻櫎鐨勬枃浠舵帶鍒跺潡瀛樺湪
            delete_dirOrFile(num2FCBBlock(B), F);
            return 1;                          //鎻愮ず寮€鍙戣€咃紝鐢卞紑鍙戣€呭憡鐭ョ敤鎴凤紝鍒犻櫎鏂囦欢鎴愬姛锛?
        }else return 0;
    }
    else
    {
        //璇存槑鎯宠鍒犻櫎鐨勬枃浠朵笉瀛樺湪锛?
        return -1;               //鎻愮ず寮€鍙戣€咃紝鐢卞紑鍙戣€呭憡鐭ョ敤鎴凤紝鍒犻櫎鐨勬枃浠朵笉瀛樺湪锛?
    }
}

//閲嶅懡鍚嶏紱鏀寔鏂囦欢鍜岀洰褰?///////////////////////////////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯(鍑芥暟灏佽濂戒簡鍚庝慨鏀瑰氨寰堟柟渚?
int sys_rename(string path, int type, string new_name) {
    //鍏堝垽璇荤敤鎴风粰瀹氱殑璺緞鏄惁鍚堟硶锛涘嵆鑳藉惁鎵惧埌杩欎釜鏂囦欢/鐩綍锛?
    //type,鏈夊紑鍙戣€呰闂敤鎴峰悗锛岃幏寰楋紱

    FCB*  parentDir = NULL;      //parentDir鐖剁洰褰曪紝鐩墠鏄笉鐭ラ亾鐨勶紝鎵€浠ヨ缃负NULL锛岄€氳繃涓嬭堪杩囩▼锛屽彲浠ョ煡閬損arentDir鏄惁瀛樺湪锛屼互鍙婃兂閲嶅懡鍚嶇殑鏂囦欢鏄惁瀛樺湪锛?
    string oldName = "";             //鐢ㄦ埛鎯宠閲嶅懡鍚嶇殑鏂囦欢鐨勫悕瀛楋紱鍦ㄤ笅闈㈠嚱鏁拌幏寰楋紱
    int B,F;
    FCB* rename_FCB = sys_returnFCB(path, oldName, type, parentDir,B,F);

    if (rename_FCB != NULL) {
        //鎯抽噸鍛藉悕鐨勬枃浠?鐩綍瀛樺湪锛?
        if (rename_FCB->_access[current_usernum] == '0' || rename_FCB->_access[current_usernum] == '1') return 0;
        else {
            //鍒ゆ柇鏂板悕瀛楁槸鍚﹀湪鐖剁洰褰曚腑閲嶅锛?
            if (returnSonFCB(parentDir, new_name, type, F, B) == NULL) {
                //璇存槑锛屾柊鍚嶅瓧娌℃湁閲嶅锛?
                strcpy(rename_FCB->_name, new_name.c_str());
                return 1;        //鎻愮ず寮€鍙戣€呴噸鍛藉悕鎴愬姛锛?
            } else {
                //鍚屽悕鍚岀被鍨嬪凡缁忓瓨鍦紝鐢变簬鏄亶鍘嗙埗鐩綍涓嬫墍鏈夌殑瀛╁瓙锛屽彲鑳芥槸鍜岃嚜宸遍噸澶嶏紝浣嗘嫆缁濋噸鍛藉悕鏁堟灉涓€鏍凤紱
                return -1;              //鎻愮ず寮€鍙戣€咃紝瀛樺湪鍚屽悕鍚岀被鍨嬶紱
            }
        }
    }
    else
    {
        //鎯抽噸鍛藉悕鐨勬枃浠朵笉瀛樺湪锛?
        if (parentDir == NULL)
        {
            //鐢变簬鐖剁洰褰曚笉瀛樺湪锛?
            return -2;       //鎻愮ず寮€鍙戣€咃紝鐖剁洰褰曚笉瀛樺湪锛?

        }
        else
        {
            return -3;         //鎻愮ず寮€鍙戣€咃紝鎯抽噸鍛藉悕鐨勬枃浠朵笉瀛樺湪锛?
        }
    }
}
//鍚戞枃浠惰鐩栧啓鍐呭锛?/////////////////////////////////////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
int sys_overwrite_file(FCB* file, string content) {
    //鏂囦欢鏄惁瀛樺湪锛屾鏃跺唴鏍镐笉鑰冭檻锛岀敱寮€鍙戣€呭鐞嗭紱
    //鎵€浠ワ紝姝ゆ椂鏂囦欢宸茬粡瀛樺湪锛?
    if (file->_access[current_usernum] == '0' || file->_access[current_usernum] == '1') return 0;
    else {
        //鏍规嵁content鍒嗛厤block涓暟锛?
        double needUsedBlock;
        int tmp;
        needUsedBlock = content.length() / _contentSize;
        tmp = content.length() % _contentSize;


        //BLOCK* contentFirstAdrr;   //鍒嗛厤鐨凚LOCK鐨勯鍦板潃锛?
        int contentFirstBlockNum;   //鍒嗛厤鐨凚LOCK鐨勯鍧楀彿锛?

        if (!tmp) {
            //璇存槑娌℃崯澶憋紝鎵€浠ュ垎閰嶇殑涓暟涓簍mp;
            contentFirstBlockNum = getEmptyBLOCKNum(needUsedBlock);


            ////if (contentFirstAdrr == NULL)
            if (contentFirstBlockNum == -1) {
                return -1;              //鎻愮ず寮€鍙戣€咃紝鎵€闇€瑕佺殑BLOCK涓嶅锛屽啓鏂囦欢澶辫触锛?
            }

        } else {

            //绮惧害鎹熷け锛涗笂鍙栨暣锛?
            needUsedBlock++;
            contentFirstBlockNum = getEmptyBLOCKNum(needUsedBlock);

            if (contentFirstBlockNum == -1) {
                return -1;              //鎻愮ず寮€鍙戣€咃紝鎵€闇€瑕佺殑BLOCK涓嶅锛屽啓鏂囦欢澶辫触锛?
            }
        }

        if (file->_block != -1) {
            releaseBLOCK(file->_block, file->_type);
        }
        file->_block = contentFirstBlockNum;
        file->_size = content.length();

        FileBLOCK *contentFirstAdrr = initFileBlock(contentFirstBlockNum);
        //鍏堝鐞嗘暣鏁板唴瀹圭殑BLOCK锛?
        for (int i = 0; i < needUsedBlock - 1; i++) {
            /////strcpy(contentFirstAdrr->_content, content.substr(i*_contentSize, _contentSize).c_str());
            strcpy(contentFirstAdrr->_content, content.substr(i * _contentSize, _contentSize).c_str());
            //content.substr(i*_contentSize, _contentSize).copy(contentFirstAdrr->_contentStart, content.substr(i*_contentSize, _contentSize).length());
            contentFirstAdrr->_contentLength = _contentSize;

            /////contentFirstAdrr = contentFirstAdrr->_nextBlock;
            contentFirstAdrr = num2FileBlock(_fat->fats[contentFirstBlockNum]);
            contentFirstBlockNum = _fat->fats[contentFirstBlockNum];//-1??????
        }
        //瀛樺彇鍓╀綑鍐呭锛?
        contentFirstAdrr->_contentLength = tmp;
        strcpy(contentFirstAdrr->_content, content.substr((needUsedBlock - 1) * _contentSize, tmp).c_str());
        //content.substr((tmp - 1)*_contentSize, contentFirstAdrr->_contentLength).copy(contentFirstAdrr->_contentStart, content.substr((tmp - 1)*_contentSize, contentFirstAdrr->_contentLength).length());
        return 1;       //鎻愮ず寮€鍙戣€咃紝鍐欐枃浠舵垚鍔燂紱

    }
}
//鍚戞枃浠惰拷鍔犲啓鍐呭锛?///////////////////////////////////////////////////淇敼瀹屾垚锛屾湭娴嬭瘯
/*
int sys_appendwrite_file(FCB* file, string content) {
    //鏂囦欢鏄惁瀛樺湪锛屾鏃跺唴鏍镐笉鑰冭檻锛岀敱寮€鍙戣€呭鐞嗭紱
    //鎵€浠ワ紝姝ゆ椂鏂囦欢宸茬粡瀛樺湪锛?
    if (file->_access[current_usernum] == '0' || file->_access[current_usernum] == '1') return 0;
        //鏍规嵁content鍒嗛厤block涓暟锛?
    else {
        double needUsedBlock, UsedBlock;
        int tmp1, tmp0;
        needUsedBlock = (content.length() + file->_size) / _contentSize;
        tmp1 = (content.length() + file->_size) % _contentSize;
        UsedBlock = file->_size / _contentSize;
        tmp0 = file->_size % _contentSize;
        if (tmp1 != 0) needUsedBlock++;
        if (tmp0 != 0) UsedBlock++;
        int newcontentFirstBlockNum;   //鏂板垎閰嶇殑BLOCK鐨勯鍧楀彿锛?
        int tmp = file->_block;;//鍘熸枃浠舵湯鍧楀彿
        int newUsedBlock = needUsedBlock - UsedBlock;//鏂板垎閰嶇殑鐩樺潡鏁?
        FileBLOCK *contentFirstAdrr;

        FileBLOCK *contentlastAdrr = num2FileBlock(tmp);

        file->_size += content.length();

        if (needUsedBlock > UsedBlock) {
            newcontentFirstBlockNum = getEmptyBLOCKNum(newUsedBlock);
            if (newcontentFirstBlockNum == -1) {
                return -1;              //鎻愮ず寮€鍙戣€咃紝鎵€闇€瑕佺殑BLOCK涓嶅锛屽啓鏂囦欢澶辫触锛?
            }
            ////if (file->_block != NULL)
            if (file->_block == -1)
                file->_block = newcontentFirstBlockNum;
            else {
                //tmp=file->_block;//feel
                //while(tmp!=-1)tmp=_fat->fats[tmp];//tmp=-1
                while (_fat->fats[tmp] != -1)tmp = _fat->fats[tmp];//_fat->fats[tmp]=-1
                _fat->fats[tmp] = newcontentFirstBlockNum;
            }
            contentFirstAdrr = initFileBlock(newcontentFirstBlockNum);
        }else{
            contentFirstAdrr=contentlastAdrr;
        }
        //鍘熸湯灏惧潡璇硶姝ｇ‘鍚????????????????
        string tmpstr = contentlastAdrr->_content;
        tmpstr += content.substr(0, _contentSize - tmp0);
        strcpy(contentlastAdrr->_content, tmpstr.c_str());
        //鍏堝鐞嗘暣鏁板唴瀹圭殑BLOCK锛?

        for (int i = 0; i < newUsedBlock - 1; i++) {
            /////strcpy(contentFirstAdrr->_content, content.substr(i*_contentSize, _contentSize).c_str());
            strcpy(contentFirstAdrr->_content,
                   content.substr(_contentSize - tmp0 + i * _contentSize, _contentSize).c_str());
            //content.substr(i*_contentSize, _contentSize).copy(contentFirstAdrr->_contentStart, content.substr(i*_contentSize, _contentSize).length());
            contentFirstAdrr->_contentLength = _contentSize;

            /////contentFirstAdrr = contentFirstAdrr->_nextBlock;
            contentFirstAdrr = num2FileBlock(_fat->fats[newcontentFirstBlockNum]);
            newcontentFirstBlockNum = _fat->fats[newcontentFirstBlockNum];//-1??????
        }
        //瀛樺彇鍓╀綑鍐呭锛?
        contentFirstAdrr->_contentLength = tmp1;
        strcpy(contentFirstAdrr->_content,content.substr(_contentSize - tmp0 + (newUsedBlock - 1) * _contentSize, tmp1).c_str());
        //content.substr((tmp - 1)*_contentSize, contentFirstAdrr->_contentLength).copy(contentFirstAdrr->_contentStart, content.substr((tmp - 1)*_contentSize, contentFirstAdrr->_contentLength).length());
        return 1;       //鎻愮ず寮€鍙戣€咃紝鍐欐枃浠舵垚鍔燂紱
    }
}*/

//璇诲彇鏂囦欢鍐呭锛?
string sys_read_file(FCB* file,bool &flag) {
    //鏂囦欢鏄惁瀛樺湪锛屾鏃跺唴鏍镐笉鑰冭檻锛岀敱寮€鍙戣€呭鐞嗭紱
    //鎵€浠ワ紝姝ゆ椂鏂囦欢宸茬粡瀛樺湪锛?
    flag=true;
    if (file->_access[current_usernum] == '0'){
        flag=false;
        return "";//NULL;
    }
    else {
        string content = "";
        ////BLOCK* tmp = file->_block;
        int tmpnum = file->_block;


        FileBLOCK *tmp = num2FileBlock(tmpnum);
        /*
        while (tmp->_nextBlock != NULL)
        {
        content += tmp->_content;
        tmp = tmp->_nextBlock;
        }
        */
        while (_fat->fats[tmpnum] != -1) {
            content += tmp->_content;
            tmpnum = _fat->fats[tmpnum];
            tmp = num2FileBlock(tmpnum);
        }
        content += ((string) tmp->_content).substr(0, tmp->_contentLength);

        //content += "\n read success!";
        return content;
    }
}

int sys_cd(string path, string &name)
{
    const int dir_type = 1;

    FCB* parent = NULL;         //瀛愮洰褰曠殑鐖剁洰褰曪紱
    FCB* subDir;        //寰呮煡鎵剧殑瀛愮洰褰曪紱
    int B,F;

    subDir = sys_returnFCB(path, name, dir_type, parent,B,F);

    if (subDir != NULL)
    {
        if (subDir->_access[current_usernum] == '0') return 0;
        else{
            _current = subDir;
            //鏇存柊褰撳墠璺緞锛?
            if (path[0] == '/')
            {
                current_path = path;
            }
            else
            {
                if (_current == _root)
                {
                    //鐩存帴鍔爌ath;
                    current_path += path;
                }
                else
                {
                    current_path += '/' + path;
                }
            }
        }

        return 1;   //鎻愮ず寮€鍙戣€咃紝杩涘叆鐩綍锛屼笉鐢ㄦ墦鍗颁俊鎭紝鏄剧ず鏁堟灉灏卞ソ锛涘ソ濂芥樉绀轰竴涓?
    }
    else
    {
        name = "鎯宠繘鍏ョ殑鐩綍涓嶅瓨鍦紒";
        return -1;       //鎻愮ず寮€鍙戣€咃紝鐩綍涓嶅瓨鍦紱
    }
}

//杩斿洖涓婁竴绾х洰褰曪紱
void sys_cdback() {
    current_path = current_path.substr(0, current_path.length() - strlen(_current->_name) );
    string dirName = "";             //鐢ㄦ埛鎯宠鏌ョ湅鐨勭洰褰曠殑鍚嶅瓧锛涘湪涓嬮潰鍑芥暟鑾峰緱锛?
    const int dir_type = 1;            //涓€瀹氭槸鐩綍锛?
    //闇€瑕佺煡閬撶敤鎴锋兂鏌ョ湅鐨勭洰褰曞瓨涓嶅瓨鍦紱
    FCB*  parentDir = NULL;      //parentDir鐖剁洰褰曪紝鐩墠鏄笉鐭ラ亾鐨勶紝鎵€浠ヨ缃负NULL锛岄€氳繃涓嬭堪杩囩▼锛屽彲浠ョ煡閬損arentDir鏄惁瀛樺湪锛屼互鍙婃兂鏌ョ湅鐨勭洰褰曟槸鍚﹀瓨鍦紱
    int B,F;
    _current = sys_returnFCB(current_path,dirName,dir_type,parentDir,B,F);
}

//鍓垏锛?
int sys_cut(string oldPath, int type, string newPath) {
    //鍏堝垽璇荤敤鎴风粰瀹氱殑璺緞鏄惁鍚堟硶锛涘嵆鑳藉惁鎵惧埌杩欎釜鏂囦欢/鐩綍锛?
    //type,newPath鏈夊紑鍙戣€呰闂敤鎴峰悗锛岃幏寰楋紱

    FCB *parentDir = NULL;      //parentDir鐖剁洰褰曪紝鐩墠鏄笉鐭ラ亾鐨勶紝鎵€浠ヨ缃负NULL锛岄€氳繃涓嬭堪杩囩▼锛屽彲浠ョ煡閬損arentDir鏄惁瀛樺湪锛屼互鍙婃兂鍓垏鐨勬枃浠舵槸鍚﹀瓨鍦紱
    string cutFCB_name;
    int Bo, Fo;
    FCB *cut_FCB = sys_returnFCB(oldPath, cutFCB_name, type, parentDir, Bo, Fo);
    //濡傛灉瑕佸壀鍒囩殑鏂囦欢FCB涓嶅瓨鍦紝閭ｄ箞杩斿洖鏂囦欢涓嶅瓨鍦?
    if (cut_FCB != NULL) {
        if (cut_FCB->_access[current_usernum] == '3' || cut_FCB->_access[current_usernum] ==
                                                        '4') {            /************************************************************************/
            //鍙壀鍒囩矘璐达紱
            //棣栧厛鍒ゆ柇锛涚矘璐村埌鐨勭洰褰曚腑锛岄噸澶嶅埄鐢╰mp锛屽垽鏂槸鍚﹀瓨鍦ㄥ悓鍚嶅悓绫诲瀷鏂囦欢/鐩綍锛?
            //parentDir锛宑utFCB_name浼氭牴鎹畃ath杩涜淇敼锛屽彲閲嶅鍒╃敤锛?
            //涓婇潰鐨刼ldPath鏄寘鎷壀鍒囩殑鏂囦欢/鐩綍鐨勶紱杩欓噷鐨刵ewPath鏄彧闇€瑕佹彁渚涜绮樿创鐨勭洰褰曞氨琛岋紱鎵€浠mp姝ゆ椂鏄釜鐖剁洰褰曪紱
            FCB *DestinationDir_parentDir = NULL;
            int Bn, Fn;
            FCB *DestinationDir = sys_returnFCB(newPath, cutFCB_name, 1, DestinationDir_parentDir, Bn,
                                                Fn);      //鏌愪釜鐩綍涓紱

            //鍒ゆ柇鐩爣鐩綍鏄惁瀛樺湪锛?
            if (DestinationDir == NULL) {
                //鐩爣鐩綍涓嶅瓨鍦紱鍒欐棤娉曢粡璐达紱
                return -4;              //鎻愮ず寮€鍙戣€咃紝鐩爣鐩綍涓嶅瓨鍦紱
            } else {
                //鐩爣鐩綍瀛樺湪锛屽垽璇诲叾涓槸鍚﹀瓨鍦ㄥ悓鍚嶅悓绫诲瀷鐨勫璞★紱
                //鏌ユ壘杩欎釜鏂囦欢|鐩綍鐨勫墠椹憋紱
                FCB *tmp;
                int Bd, Fd;
                tmp = returnSonFCB(DestinationDir, cut_FCB->_name, cut_FCB->_type, Bd, Fd);
                if (tmp == NULL) {
                    //璇存槑涓嶅瓨鍦ㄥ悓鍚嶅悓绫诲瀷鐨勫璞★紱鍙互榛忚创锛涙憳闄や箣鍓嶇洰褰曟爲鐨勭粨鐐?
                    add_dirOrFile(cut_FCB, DestinationDir);
                    delete_dirOrFile(num2FCBBlock(Bo), Fo);

                    return 1;          //鎻愮ず寮€鍙戣€咃紝鍓垏绮樿创鎴愬姛锛?
                } else {
                    return -1;       //鎻愮ず寮€鍙戣€咃紝璇ョ洰褰曚笅瀛樺湪鍚屽悕鍚岀被鍨嬶紝涓嶅厑璁哥矘璐达紱
                }
            }

        } else return 0;
    }
    else
    {
        if (parentDir == NULL)
        {
            return -2;          //鎻愮ず寮€鍙戣€咃紝鎯宠鍓垏鐨勬枃浠剁殑鐖剁洰褰曚笉瀛樺湪锛?
        }
        else
        {
            //璇存槑鎯宠鍓垏鐨勬枃浠朵笉瀛樺湪锛?
            return -3;               //鎻愮ず寮€鍙戣€咃紝鐢卞紑鍙戣€呭憡鐭ョ敤鎴凤紝鍓垏鐨勬枃浠朵笉瀛樺湪锛?
        }
    }
}

//澶嶅埗锛?
int sys_copy(string oldPath, int type, string newPath) {
    //鍏堝垽璇荤敤鎴风粰瀹氱殑璺緞鏄惁鍚堟硶锛涘嵆鑳藉惁鎵惧埌杩欎釜鏂囦欢/鐩綍锛?
    //type,鏈夊紑鍙戣€呰闂敤鎴峰悗锛岃幏寰楋紱

    FCB *parentDir = NULL;      //parentDir鐖剁洰褰曪紝鐩墠鏄笉鐭ラ亾鐨勶紝鎵€浠ヨ缃负NULL锛岄€氳繃涓嬭堪杩囩▼锛屽彲浠ョ煡閬損arentDir鏄惁瀛樺湪锛屼互鍙婃兂鍓垏鐨勬枃浠舵槸鍚﹀瓨鍦紱
    string copyFCB_name;
    int Bo, Fo;
    FCB *copy_FCB = sys_returnFCB(oldPath, copyFCB_name, type, parentDir, Bo, Fo);

    if (copy_FCB != NULL) {
        if (copy_FCB->_access[current_usernum] == '3' || copy_FCB->_access[current_usernum] == '4') {
            //鎯宠澶嶅埗鐨勫瓨鍦紱
            //澶嶅埗鏃讹紝鑾峰彇瑕佸鍒堕鍦板潃鍗冲彲锛屼絾绮樿创瑕佹敞鎰忥紝鎷疯礉涓€浠藉拰鍘熸潵涓€鏍风殑锛屽鍔燜CB寮€閿€锛宐lock寮€閿€锛涗笖鐩綍鐨刡rother瑕佺疆绌猴紱
            //_copyNode = copy_FCB;


            /**********************************************************/
            //杩涘叆澶嶅埗绮樿创锛?
            int Bn, Fn;
            //鍏堝垽鏂澶嶅埗鍒扮殑閭ｄ釜鐩綍涓紝鏄惁瀛樺湪鍚屽悕鍚岀被鍨嬬殑鏂囦欢锛?
            //copyFCB_name鏄函绮规墦閰辨补鐨勶紱
            FCB *DestinationDir = sys_returnFCB(newPath, copyFCB_name, 1, parentDir, Bn, Fn);      //鏌愪釜鐩綍涓紱
            if (DestinationDir == NULL) {//鐩爣鐩綍涓嶅瓨鍦紱鏃犳硶杩涜澶嶅埗绮樿创锛?
                return -5;           //鎻愮ず寮€鍙戣€呯洰鏍囩洰褰曚笉瀛樺湪锛?
            } else {
                //鐩爣鐩綍瀛樺湪锛?
                //杩涘叆鐩爣鐩綍锛屽垽鏂槸鍚﹀瓨鍦ㄥ悓鍚嶅悓绫诲瀷鐨勫璞★紱
                FCB *tmp;
                int Bd, Fd;
                tmp = returnSonFCB(DestinationDir, copy_FCB->_name, copy_FCB->_type, Bd, Fd);
                if (tmp == NULL) {
                    //鐢ㄦ埛鎯冲鍒跺埌鐨勭洰褰曚腑娌℃湁鍚屽悕鍚岀被鍨嬶紱
                    //鏈川灏辨槸澧炲姞FCB锛屽凡缁廈lLOCK;
                    //鐢变簬璧勬簮鏈夐檺锛屾墍浠ラ渶鍒ゆ柇涓€涓渶瑕佸鍔犵殑FCB鍜孊LOCK澶熶笉澶燂紱

                    //numFCB = 0; numBLOCK = 0;

                    //璋冪敤璁＄畻绯荤粺璧勬簮锛?
                    int numBLOCK = num_dirOrFile(copy_FCB);

                    if (numBLOCK <= _emptyBLOCK_Count) {
                        //璇存槑绯荤粺璧勬簮澶熺敤锛涘彲浠ュ鍒讹紱
                        add_dirOrFile(copy_FCB, DestinationDir);
                        return 1;          //鎻愮ず寮€鍙戣€咃紝 澶嶅埗绮樿创鎴愬姛锛?
                    } else {
                        return -1;     //鎻愮ず寮€鍙戣€咃紝绯荤粺璧勬簮涓嶅锛屾棤娉曡繘琛屽鍒讹紱
                    }

                } else {
                    return -2;       //鎻愮ず寮€鍙戣€咃紝璇ョ洰褰曚笅瀛樺湪鍚屽悕鍚岀被鍨嬶紝涓嶅厑璁哥矘璐达紱
                }
            }
        } else return 0;

    } else {
        if (parentDir == NULL) {
            return -3;          //鎻愮ず寮€鍙戣€咃紝鎯宠澶嶅埗鐨勬枃浠剁殑鐖剁洰褰曚笉瀛樺湪锛?
        } else {
            //璇存槑鎯宠澶嶅埗鐨勬枃浠朵笉瀛樺湪锛?
            return -4;               //鎻愮ず寮€鍙戣€咃紝鐢卞紑鍙戣€呭憡鐭ョ敤鎴凤紝澶嶅埗鐨勬枃浠朵笉瀛樺湪锛?
        }
    }
}

int sys_share(string oldPath, int type, string newPath) {
    //鍏堝垽璇荤敤鎴风粰瀹氱殑璺緞鏄惁鍚堟硶锛涘嵆鑳藉惁鎵惧埌杩欎釜鏂囦欢/鐩綍锛?
    //type,鏈夊紑鍙戣€呰闂敤鎴峰悗锛岃幏寰楋紱

    FCB *parentDir = NULL;      //parentDir鐖剁洰褰曪紝鐩墠鏄笉鐭ラ亾鐨勶紝鎵€浠ヨ缃负NULL锛岄€氳繃涓嬭堪杩囩▼锛屽彲浠ョ煡閬損arentDir鏄惁瀛樺湪锛屼互鍙婃兂鍓垏鐨勬枃浠舵槸鍚﹀瓨鍦紱
    string copyFCB_name;
    int Bo, Fo;
    FCB *share_FCB = sys_returnFCB(oldPath, copyFCB_name, type, parentDir, Bo, Fo);

    if (share_FCB != NULL) {
        if (share_FCB->_access[current_usernum] == '3' || share_FCB->_access[current_usernum] == '4') {
            //鎯宠澶嶅埗鐨勫瓨鍦紱
            //澶嶅埗鏃讹紝鑾峰彇瑕佸鍒堕鍦板潃鍗冲彲锛屼絾绮樿创瑕佹敞鎰忥紝鎷疯礉涓€浠藉拰鍘熸潵涓€鏍风殑锛屽鍔燜CB寮€閿€锛宐lock寮€閿€锛涗笖鐩綍鐨刡rother瑕佺疆绌猴紱
            //_copyNode = copy_FCB;

            /**********************************************************/
            //杩涘叆澶嶅埗绮樿创锛?
            int Bn, Fn;
            //鍏堝垽鏂澶嶅埗鍒扮殑閭ｄ釜鐩綍涓紝鏄惁瀛樺湪鍚屽悕鍚岀被鍨嬬殑鏂囦欢锛?
            //copyFCB_name鏄函绮规墦閰辨补鐨勶紱
            FCB *DestinationDir = sys_returnFCB(newPath, copyFCB_name, 1, parentDir, Bn, Fn);      //鏌愪釜鐩綍涓紱
            if (DestinationDir == NULL) {//鐩爣鐩綍涓嶅瓨鍦紱鏃犳硶杩涜澶嶅埗绮樿创锛?
                return -4;           //目标不存在
            }
            else {
                //鐩爣鐩綍瀛樺湪锛?
                //杩涘叆鐩爣鐩綍锛屽垽鏂槸鍚﹀瓨鍦ㄥ悓鍚嶅悓绫诲瀷鐨勫璞★紱
                FCB *tmp;
                int Bd, Fd;
                tmp = returnSonFCB(DestinationDir, share_FCB->_name, share_FCB->_type, Bd, Fd);
                if (tmp == NULL) {
                    //鐢ㄦ埛鎯冲鍒跺埌鐨勭洰褰曚腑娌℃湁鍚屽悕鍚岀被鍨嬶紱
                    //鏈川灏辨槸澧炲姞FCB锛屽凡缁廈lLOCK;
                    //鐢变簬璧勬簮鏈夐檺锛屾墍浠ラ渶鍒ゆ柇涓€涓渶瑕佸鍔犵殑FCB鍜孊LOCK澶熶笉澶燂紱

                    //numFCB = 0; numBLOCK = 0;

                    //璋冪敤璁＄畻绯荤粺璧勬簮锛?
                        //璇存槑绯荤粺璧勬簮澶熺敤锛涘彲浠ュ鍒讹紱
                    int s=share_dirOrFile(share_FCB, DestinationDir);
                    if(s==1) return 1;          //鎻愮ず寮€鍙戣€咃紝 澶嶅埗绮樿创鎴愬姛锛?
                    else return 0;

                } else
                    return -1;//存在同名

            }
        } else return 0;

    } else {
        if (parentDir == NULL) {
            return -2;          //鎻愮ず寮€鍙戣€咃紝鎯宠澶嶅埗鐨勬枃浠剁殑鐖剁洰褰曚笉瀛樺湪锛?
        } else {
            //璇存槑鎯宠澶嶅埗鐨勬枃浠朵笉瀛樺湪锛?
            return -3;   //共享文件不存在
        }
    }
}







int sys_setaccess(string path,char x){
    string name1;
    int type,B,F;
    FCB *p;
    FCB* fcb=sys_returnFCB(path,name1,0,p,B,F);
    if(fcb->_access[current_usernum]=='4') {
        int i=0;
        while(i< _maxUsers){
            if(i!=0||i!=current_usernum)
                fcb->_access[current_usernum]=x;
            i++;
        }
        return 1;
    }else return 0;
}
/*
int main() {
    sys_initDisk();
    string name1;
    int type,B,F;
    FCB *p;
    sys_mkdir("/m") ;

    //if(sys_cd("/m",name)==1) cout<<"cd"<<endl;
    sys_create_file("/m/t");
    FCB* file= sys_returnFCB("/m/t",name1,0,p,B,F);
    if (sys_overwrite_file(file,"hello") == 1) ;
    sys_copy("/m/t",0,"/");

    FCB* file1= sys_returnFCB("/t",name1,0,p,B,F);
    //cout<<sys_read_file(file1);
    //sys_appendwrite_file(file1,"world");
    //cout<<sys_read_file(file1);
    string name;
    //sys_rmdir("/m");
    ///sys_delete_file("/m/t");
//    cout<<sys_read_file(file);
//    StringList* ss=sys_dir("");
//    cout<<ss->content<<endl;
//    cout<<ss->next->content<<endl;

    if(sys_createuser("Jiang")==1)cout<<"user";
    if(sys_su("Jiang")==1) cout<<"su";
    bool flag;
    string s=sys_read_file(file1,flag);
    if(flag)cout<<s;
    else cout<<"fobidden";

}*/

int sys_write_disk(string diskpath){
    ofstream write_disk;
    if(diskpath.empty()){
        write_disk.open("virtualDisk.bin", ios::ate | ios::out | ios::binary);
    } else {
        write_disk.open(diskpath.c_str(), ios::ate | ios::out | ios::binary);
    }

    if(!write_disk.is_open()){
        return -1;
    } else {
        write_disk.write(_virtualDiskAddr,_virtualDiskSize);
        write_disk.close();
        return 1;
    }
}

int sys_read_disk(string diskpath){
    ifstream read_disk;
    if(diskpath.empty()){
        read_disk.open("virtualDisk.bin", ios::in | ios::binary);
    } else {
        read_disk.open(diskpath.c_str(), ios::in | ios::binary);
    }

    if(!read_disk.is_open()){
        return -1;
    } else {
        read_disk.read(_virtualDiskAddr,_virtualDiskSize);
        //关文件；
        read_disk.close();
        return 1;
    }
}
