
create table t_file_info(
	id int not null primary key,
	chk int,
	version varchar(50) not null,
	filepath varchar(100),
	softtype int
);

insert into t_file_info values(1,1,"Nabao_VS980.2008","/home/linden/workspace/PosServer/fileList/Nabao_VS980.2008_20150128_AB9A022D.bin",1);

insert into t_file_info values(2,1,"LianRong_VS981.5","/home/linden/workspace/PosServer/fileList/LianRong_VS981.5_20150112_3B2FB261.bin",1);
/*
create table t_update_snlist(
	id int not null primary key,
	sn varchar(50),
	terVersion varchar(50),
	userVersion varchar(50),
	bootVersion varchar(50),
	terCrc varchar(50),
	softCrc varchar(20),
	checkTime datetime,
	startTime datetime,
	endTime datetime,
	npack varchar(10),
	nsoftverid varchar(10),
	nstate varchar(10),
	cusname varchar(255)
);

insert into t_update_snlist (id,sn,userVersion,softCrc,nsoftverid, nstate) values (7, "S980114-11079508","LianRong_VS981.5","3B2FB261","1","0");

insert into t_update_snlist (id,sn,userVersion,softCrc,nsoftverid, nstate) values (2, "S980114-00000004","Nabao_VS980.2008","ab9a022d","1","0");

insert into t_update_snlist (id,sn,userVersion,softCrc,nsoftverid, nstate) values (10, "S980114-09090909","Nabao_VS980.2008","ab9a022d","1","0");
*/

/*
yh_system  系统参数表 配置端口号等信息  nConType 下载类型 


t_file_info 软件信息列表 
chk 当前软件是否使用  
version 版本号 
filepath 软件在服务器路径
findex  软件索引,增加SN的时候软件列表有当前索引 
nsofttype软件类型  3 为MOMID升级 其他为软件升级


t_check_sn_list  SN号论证远程切机使
SeqNum SN号
Sstate 是否被使用
Logintime 使用时间
softversion 软件版本
upTime 论证时间
其他字段未使用

t_sn_man 生产测试使用
ChipData 芯片ID
SeqNum SN号
Scrc 软件CRC
LoginTime 测试时间
UpTime 重测时间
bootversion BOOT版本号
其他字段未使用

t_update_snlist SN号升级列表
sn SN号
terversion 终端版本号
userversion 用户设置版本
bootversion BOOT版本
tercrc 终端CRC
softcrc 本地升级软件CRC
LoginTime 检测更新时间
UpStartTime 开始升级时间
UpEndTime 升级成功时间
npack 当前升级包的个数(主要是给前端使用的)
nsoftverid  升级软件ID 对应 t_file_list 中的 findex
cusname 暂时不用
nstate 升级状态 0  未升级 1升级中 2升级成功




在添加SN号时,如果SQL中有这个SN号,如何删除旧的添加新的

*/
