
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
yh_system  ϵͳ������ ���ö˿ںŵ���Ϣ  nConType �������� 


t_file_info �����Ϣ�б� 
chk ��ǰ����Ƿ�ʹ��  
version �汾�� 
filepath ����ڷ�����·��
findex  �������,����SN��ʱ������б��е�ǰ���� 
nsofttype�������  3 ΪMOMID���� ����Ϊ�������


t_check_sn_list  SN����֤Զ���л�ʹ
SeqNum SN��
Sstate �Ƿ�ʹ��
Logintime ʹ��ʱ��
softversion ����汾
upTime ��֤ʱ��
�����ֶ�δʹ��

t_sn_man ��������ʹ��
ChipData оƬID
SeqNum SN��
Scrc ���CRC
LoginTime ����ʱ��
UpTime �ز�ʱ��
bootversion BOOT�汾��
�����ֶ�δʹ��

t_update_snlist SN�������б�
sn SN��
terversion �ն˰汾��
userversion �û����ð汾
bootversion BOOT�汾
tercrc �ն�CRC
softcrc �����������CRC
LoginTime ������ʱ��
UpStartTime ��ʼ����ʱ��
UpEndTime �����ɹ�ʱ��
npack ��ǰ�������ĸ���(��Ҫ�Ǹ�ǰ��ʹ�õ�)
nsoftverid  �������ID ��Ӧ t_file_list �е� findex
cusname ��ʱ����
nstate ����״̬ 0  δ���� 1������ 2�����ɹ�




�����SN��ʱ,���SQL�������SN��,���ɾ���ɵ�����µ�

*/
