#include "sh_trsf_sjd.h"
#include"string"
#include<qstring.h>
#include"qdatetime.h"
#include"qfile.h"
#include"qtextstream.h"
#include"qjsondocument.h"
#include "qdebug.h"
#include "qcoreapplication.h"
#include"qdir.h"

using namespace  std;

//��ȡҵ���
int GetServiceTypeID()
{
	return 8;
}

//��ȡҵ������
QString GetServiceTypeName()
{
	QString name = QString::fromLocal8Bit("ʪ����������ҵ��");
	return name;
}

//��ȡ�汾��
QString GetVersionNo()
{
	QString Version = QString::fromLocal8Bit("1.0");
	return Version;
}

//����ʹ����¼���Ӧ
void  GetControlWidget(QString StationID, uint Socket, QWidget* parent) //<>
{
	if (isActive)
		return;
	w = new SetParamDlg(parent);
	w->StationID = StationID;
	isActive = true;
	w->Socket = Socket;
	w->isActive = &isActive;
	w->show();
}

//��ȡ�˿ں�
int GetPort()
{
	return 23007;
}

//����ʱ��
void SetTime(QString StationID, uint Socket)
{
	//��ȡ��ǰʱ��
	QDateTime nowtime = QDateTime::currentDateTime();
	QString datetime = nowtime.toString("yyyy.MM.dd hh:mm:ss");
	QString year, month, day, hour, min, sec;
	year = nowtime.toString("yy");
	month = nowtime.toString("MM");
	day = nowtime.toString("dd");
	hour = nowtime.toString("hh");
	min = nowtime.toString("mm");
	sec = nowtime.toString("ss");
	//����ʱ��
	int chk = 0;
	int SrcAdrr = StationID.toInt();
	BYTE bytes[1024] = { 0 };
	bytes[0] = 0xaa;
	bytes[1] = 0x0a;//֡����
	bytes[2] = 0x81;//֡����
	chk += bytes[2];
	bytes[3] = SrcAdrr & 0xff;//Դ��ַ
	chk += bytes[3];
	bytes[4] = (SrcAdrr >> 8) & 0xff;
	chk += bytes[4];
	bytes[5] = 0x14;//20
	chk += bytes[5];
	bytes[6] = year.toInt();
	chk += bytes[6];
	bytes[7] = month.toInt();
	chk += bytes[7];
	bytes[8] = day.toInt();
	chk += bytes[8];
	bytes[9] = hour.toInt();
	chk += bytes[9];
	bytes[10] = min.toInt();
	chk += bytes[10];
	bytes[11] = sec.toInt();
	chk += bytes[11];
	bytes[12] = chk & 0xff;//У��λ �Ͱ�λ
	bytes[13] = (chk >> 8) & 0xff;//�߰�λ
	bytes[14] = 0xdd;
	::send(Socket, (char *)bytes, 15, 0);
}

//����ֵ����
void SetValueToControlWidget(QStringList list)
{
	if (w == nullptr)
		return;
	if (isActive)
		w->setValue(list);
}

//��������
LRESULT Char2Json(QString &buff, QJsonObject &json)
{
	int Count = 0;//���ݸ���
	int Current_P = buff.length();//��ǰ����ָ

								  //������������
	for (int i = 0; i < buff.length(); i++)
	{
		//֡ͷ fe fe fe aa
		if (buff[i] == 0xaa)
		{
			Current_P = i;//ָ��ָ��֡ͷ
			for (int j = i + 1; j < buff.length(); j++)
			{
				if (buff[j] == 0xdd)
				{
					Current_P = j + 1;//ָ���ƶ���֡β��һ���ַ�

					QString strBuff = buff.mid(i, j - i + 1);
					QJsonObject SubJson;
					SubJson.insert("ServiceTypeID", SH_TRSF_SJD);

					Frame frame = { 0,NULL,NULL };
					//��ȡ֡����
					frame.len = ((strBuff[1]).unicode() & 0xFF) - 3;
					//�ж�֡����
					if (strBuff.length() - frame.len != 8)
						continue;
					//��ȡ֡����
					frame.Command = strBuff[2].unicode();
					//��ȡԴ��ַ
					frame.SrcAddr = strBuff[3].unicode();
					//��ȡĿ�ĵ�ַ
					frame.DesAddr = strBuff[4].unicode();
					if ((strBuff[4].unicode() == 0x01) || (strBuff[4].unicode() == 0x0A))
						frame.SrcAddr = strBuff[3].unicode();
					else
						frame.SrcAddr = 256 * strBuff[4].unicode() + strBuff[3].unicode();
					//��վ��
					SubJson.insert("StationID","NULL");
					//�豸��
					SubJson.insert("DeviceID", QString::number(frame.SrcAddr));
					//���ݴ洢����
					SubJson.insert("DataSourceID", 16);
					frame.data = strBuff.mid(5, frame.len);
					Count += 1;//���ݸ���
							   //�жϽ�����������
					switch (frame.Command)
					{
						//��ȡ�ɼ�ʱ�ӷ������� 0x2
					case 2:
						Transform2Time(frame.data, SubJson);
						break;
						//ͨ��״̬
					case 30:
						Transform2IPChannel(frame.data, SubJson);
						break;
						//����IP��ַ�Ͷ˿�
					case 31:
						Transform2IPPort(frame.data, SubJson);
						break;
						//������APN��ַ 0x20
					case 32:
						Transform2APNAddr(frame.data, SubJson);
						break;
						//GPRSƽ�������ˮ����Ƶ�����ݴ��¶� 0x0204
					case 114:
						Transform2GPRSPerVolumeTmp(frame.data, SubJson);
						break;
						//���������� 0x3A
					case 58:
						Transform2Heartbeat(frame.data, SubJson);
						break;
						//ͨѶ���� 0x0f
					case 15:
						Transform2CloeseConnection(frame.data, SubJson);
						break;
					default:
						SubJson.insert("Command", frame.Command);
						break;
					}
					//���ݱ���
					QDateTime current_date_time = QDateTime::currentDateTime();
					QString current_date = current_date_time.toString("yyyy.MM.dd hh:mm:ss");
					QString current_day = current_date_time.toString("yyyy-MM-dd");
					QString fileName = QCoreApplication::applicationDirPath() + "\\SH\\TRSF_SJD\\" + QString::number(frame.SrcAddr) + "\\" + current_day;
					QDir dir(fileName);
					if (!dir.exists())
						dir.mkpath(fileName);//�����༶Ŀ¼
					fileName += "\\data.txt";
					QFile file(fileName);

					if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append))
					{
					}
					QTextStream in(&file);

					QString ConvStr;
					for (int i = 0; i < strBuff.length(); i++)
					{
						ConvStr += (QString::number(strBuff[i].unicode(), 16)) + " ";
					}
					in << current_date << "\r\n" << ConvStr << "\r\n";
					file.close();
					json.insert(QString::number(Count), SubJson);
					i = j;//��ǰѭ��
					break;
				}
			}
		}
	}
	json.insert("DataLength", Count);//JSON���ݸ���
	if (Current_P >= buff.length())//�жϵ�ǰָ��λ��
	{
		buff.clear();
	}//����ڴ�
	else
	{
		buff.remove(0, Current_P);
	}//��ʣ���ֽڴ��뻺��
	return 1;
}

QString Convert2Time(QString strTime)
{
	QString tmp;
	tmp = strTime.mid(0, 4) + "-" + strTime.mid(4, 2) + "-" + strTime.mid(6, 2) + " " + strTime.mid(8, 2) + ":" + strTime.mid(10, 2) + ":" + strTime.mid(12, 2);
	return tmp;
}

//�ɼ�ʱ�ӷ�������
void Transform2Time(QString data, QJsonObject &json)
{
	json.insert("DataType", 2);//�������� �۲�����
	json.insert("ValueCount", 1);//����ֵ����
								 //ʱ��
	QString year = QString("20%1").arg((int)data[0].unicode(), 2, 10, QChar('0'));
	QString month = QString("%1").arg((int)data[1].unicode(), 2, 10, QChar('0'));
	QString day = QString("%1").arg((int)data[2].unicode(), 2, 10, QChar('0'));
	QString hour = QString("%1").arg((int)data[3].unicode(), 2, 10, QChar('0'));
	QString min = QString("%1").arg((int)data[4].unicode(), 2, 10, QChar('0'));
	QString time = Convert2Time(year + month + day + hour + min + "00");
	json.insert("RecvValue1", time);
}

//IP��ַ�Ͷ˿ں�
void Transform2IPPort(QString data, QJsonObject &json)
{
	json.insert("DataType", 2);//�������� �۲�����
	json.insert("ValueCount", 2);//����ֵ����

								 //��ȡIP
	QString IP;
	IP.sprintf("%d.%d.%d.%d", (int)data[1].unicode(), (int)data[2].unicode(), (int)data[3].unicode(), (int)data[4].unicode());
	json.insert("RecvValue1", IP);

	//��ȡ�˿�
	int Port = data[5].unicode();
	Port |= (data[6].unicode() << 8);
	json.insert("RecvValue2", QString::number(Port));
}

//IPͨ��״̬
void Transform2IPChannel(QString data, QJsonObject &json)
{
	json.insert("DataType", 2);//�������� �۲�����
	json.insert("ValueCount", 2);//����ֵ����

								 //��ȡIP
	int Channel;
	Channel=data[0].unicode();
	json.insert("RecvValue1", QString::number(Channel));

	//��ȡͨ��״̬
	int status = data[1].unicode();
	json.insert("RecvValue2", QString::number(status));
}

//������APN��ַ
void Transform2APNAddr(QString data, QJsonObject &json)
{
	json.insert("DataType", 2);//�������� �۲�����
	json.insert("ValueCount", 1);//����ֵ����

    //��ȡAPN
	json.insert("RecvValue2", data);
}

//GPRS����Сʱƽ�������ˮ����Ƶ������
void Transform2GPRSPerVolumeTmp(QString data, QJsonObject &json)
{
	json.insert("DataType", 1);//�������� �۲�����
							   //bytesתfloat
	FLOAT_UNION  f;
	//ʱ��
	QString year = QString("20%1").arg((int)data[0].unicode(), 2, 10, QChar('0'));
	QString month = QString("%1").arg((int)data[1].unicode(), 2, 10, QChar('0'));
	QString day = QString("%1").arg((int)data[2].unicode(), 2, 10, QChar('0'));
	QString hour = QString("%1").arg((int)data[3].unicode(), 2, 10, QChar('0'));
	QString min = QString("%1").arg((int)data[4].unicode(), 2, 10, QChar('0'));
	QString time = Convert2Time(year + month + day + hour + min + "00");
	json.insert("ObserveTime", time);

	//��ȡ��ѹֵ������
	f.float_byte.low_byte = data[5].unicode();
	f.float_byte.mlow_byte = data[6].unicode();
	f.float_byte.mhigh_byte = data[7].unicode();
	f.float_byte.high_byte = data[8].unicode();
	json.insert("MainClctrVltgVal", QJsonValue(f.value));

	//����ˮ��
	f.float_byte.low_byte = data[9].unicode();
	f.float_byte.mlow_byte = data[10].unicode();
	f.float_byte.mhigh_byte = data[11].unicode();
	f.float_byte.high_byte = data[12].unicode();
	json.insert("SOILVOLUME10", QJsonValue(f.value));

	//�����¶�
	f.float_byte.low_byte = data[13].unicode();
	f.float_byte.mlow_byte = data[14].unicode();
	f.float_byte.mhigh_byte = data[15].unicode();
	f.float_byte.high_byte = data[16].unicode();
	json.insert("ST_ST1", QJsonValue(f.value));

	//�����絼��
	f.float_byte.low_byte = data[17].unicode();
	f.float_byte.mlow_byte = data[18].unicode();
	f.float_byte.mhigh_byte = data[19].unicode();
	f.float_byte.high_byte = data[20].unicode();
	json.insert("EC", QJsonValue(f.value));

	//PHֵ
	f.float_byte.low_byte = data[21].unicode();
	f.float_byte.mlow_byte = data[22].unicode();
	f.float_byte.mhigh_byte = data[23].unicode();
	f.float_byte.high_byte = data[24].unicode();
	json.insert("PH", QJsonValue(f.value));

	//��ȡGPRS�ź�ǿ��
	int PowerOfGPRS = data[25].unicode();
	json.insert("SignalStrength", PowerOfGPRS);

	//��ȡGPRS������
	int GPRSBER = data[26].unicode();
	json.insert("ErrorRate", GPRSBER);
}

//���ݴ���ָ��
void Transform2ErrorCommand(QString data, QJsonObject &json)
{

}

//���յ�������
void Transform2Heartbeat(QString data, QJsonObject &json)
{
	json.insert("DataType", 3);//�������� 3��������

}

//ͨ�Ž���
void Transform2CloeseConnection(QString data, QJsonObject &json)
{
	json.insert("DataType", 2);//�������� �۲�����
	json.insert("ValueCount", 1);
	json.insert("RecvValue1", QString::fromLocal8Bit("ͨ�Ž���"));
}

//�ն�����
void SetCommand(uint Socket, int CommandType, QString Params1, QString Params2, QString StationID)
{
	switch (CommandType)
	{
	case 1601:
		//����ʱ��
	{
		//��ȡ��ǰʱ��
		QDateTime nowtime = QDateTime::currentDateTime();
		QString datetime = nowtime.toString("yyyy.MM.dd hh:mm:ss");
		QString year, month, day, hour, min, sec;
		year = nowtime.toString("yy");
		month = nowtime.toString("MM");
		day = nowtime.toString("dd");
		hour = nowtime.toString("hh");
		min = nowtime.toString("mm");
		sec = nowtime.toString("ss");
		//����ʱ��
		int chk = 0;
		int SrcAdrr = StationID.toInt();
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 0x0a;//֡����
		bytes[2] = 0x81;//֡����
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];
		bytes[5] = 0x14;//20
		chk += bytes[5];
		bytes[6] = year.toInt();
		chk += bytes[6];
		bytes[7] = month.toInt();
		chk += bytes[7];
		bytes[8] = day.toInt();
		chk += bytes[8];
		bytes[9] = hour.toInt();
		chk += bytes[9];
		bytes[10] = min.toInt();
		chk += bytes[10];
		bytes[11] = sec.toInt();
		chk += bytes[11];
		bytes[12] = chk & 0xff;//У��λ �Ͱ�λ
		bytes[13] = (chk >> 8) & 0xff;//�߰�λ
		bytes[14] = 0xdd;
		::send(Socket, (char *)bytes, 15, 0);
	}
	break;
	case 1602:
		//��ʱ��
	{
		int chk = 0;
		int SrcAdrr = StationID.toInt();
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 0x03;//֡����
		bytes[2] = 0x82;//֡����
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];
		bytes[5] = chk & 0xff;//У��λ �Ͱ�λ
		bytes[6] = (chk >> 8) & 0xff;//�߰�λ
		bytes[7] = 0xdd;
		::send(Socket, (char *)bytes, 8, 0);
		break;
	}
	case 1603:
		//��������
	{
		QDateTime Time_B, Time_E;
		Time_B = QDateTime::fromString(Params1, "yyyy-MM-dd hh:mm:ss");
		Time_E = QDateTime::fromString(Params2, "yyyy-MM-dd hh:mm:ss");

		QString yearB, monthB, dayB, hourB, minB, yearE, monthE, dayE, hourE, minE;
		yearB = Time_B.toString("yy");;
		monthB = Time_B.toString("MM");
		dayB = Time_B.toString("dd");
		hourB = Time_B.toString("hh");
		minB = Time_B.toString("mm");

		yearE = Time_E.toString("yy");//����ʱ��
		monthE = Time_E.toString("MM");
		dayE = Time_E.toString("dd");
		hourE = Time_E.toString("hh");
		minE = Time_E.toString("mm");
		int chk = 0;
		int SrcAdrr = StationID.toInt();
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 0x0d;//֡����
		bytes[2] = 0x94;//֡����
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];
		bytes[5] = yearB.toInt() - 2000;
		chk += bytes[5];
		bytes[6] = monthB.toInt();
		chk += bytes[6];
		bytes[7] = dayB.toInt();
		chk += bytes[7];
		bytes[8] = hourB.toInt();
		chk += bytes[8];
		bytes[9] = minB.toInt();
		chk += bytes[9];
		bytes[10] = yearE.toInt() - 2000;
		chk += bytes[10];
		bytes[11] = monthE.toInt();
		chk += bytes[11];
		bytes[12] = dayE.toInt();
		chk += bytes[12];
		bytes[13] = hourE.toInt();
		chk += bytes[13];
		bytes[14] = minE.toInt();
		chk += bytes[14];
		bytes[15] = chk & 0xff;//У��λ �Ͱ�λ
		bytes[16] = (chk >> 8) & 0xff;//�߰�λ
		bytes[17] = 0xdd;
		::send(Socket, (char *)bytes, 18, 0);
		break;
	}
	case 1604:
		//ͨ��״̬
	{
		int chk = 0;
		int SrcAdrr = StationID.toInt();
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 0x05;//֡����
		bytes[2] = 0x9d;//֡����
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];
		bytes[5] = 0x00;
		chk += bytes[5];
		bytes[6] = Params1.toInt();
		chk += bytes[6];
		bytes[7] = chk & 0xff;//У��λ �Ͱ�λ
		bytes[8] = (chk >> 8) & 0xff;//�߰�λ
		bytes[9] = 0xdd;
		::send(Socket, (char *)bytes, 10, 0);
	}
	break;
	case 1605:
		//��ͨ��
	{
		//��ȡ��ǰͨ��
		int channel = Params1.toInt();
		//��ͨ��
		int chk = 0;
		int SrcAdrr = StationID.toInt();
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 0x05;//֡����
		bytes[2] = 0x9c;//֡����
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];
		bytes[5] = channel;
		chk += bytes[5];
		bytes[6] = 0x1;
		chk += bytes[6];
		bytes[7] = chk & 0xff;//У��λ �Ͱ�λ
		bytes[8] = (chk >> 8) & 0xff;//�߰�λ
		bytes[9] = 0xdd;
		::send(Socket, (char *)bytes, 10, 0);
		break;
	}
	case 1606:
		//�ر�ͨ��
	{
		int channel = Params1.toInt();

		int chk = 0;
		int SrcAdrr = StationID.toInt();
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 0x05;//֡����
		bytes[2] = 0x9c;//֡����
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];
		bytes[5] = channel;
		chk += bytes[5];
		bytes[6] = 0x0;
		chk += bytes[6];
		bytes[7] = chk & 0xff;//У��λ �Ͱ�λ
		bytes[8] = (chk >> 8) & 0xff;//�߰�λ
		bytes[9] = 0xdd;
		::send(Socket, (char *)bytes, 10, 0);
		break;
	}
	case 1607:
		//����IP
	{
		int channel = Params1.toInt();

		int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0, port = 0;
		QList<QString> iplist = Params2.split('.');
		ip1 = iplist[0].toInt();
		ip2 = iplist[1].toInt();
		ip3 = iplist[2].toInt();
		ip4 = iplist[3].toInt();
		port = iplist[4].toInt();
		int chk = 0;
		int SrcAdrr = StationID.toInt();
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 0x10;//֡����
		bytes[2] = 0x8e;//֡����
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];
		bytes[5] = channel;//ͨ��
		chk += bytes[5];
		bytes[6] = ip1;//IP
		chk += bytes[6];
		bytes[7] = ip2;//IP
		chk += bytes[7];
		bytes[8] = ip3;//IP
		chk += bytes[8];
		bytes[9] = ip4;//IP
		chk += bytes[9];
		bytes[10] = port & 0xff;//port
		chk += bytes[10];
		bytes[11] = (port >> 8) & 0xff;//port
		chk += bytes[11];
		bytes[12] = chk & 0xff;//У��λ �Ͱ�λ
		bytes[13] = (chk >> 8) & 0xff;//�߰�λ
		bytes[14] = 0xdd;
		::send(Socket, (char *)bytes, 15, 0);
		break;
	}
	case 1608:
		//��ȡIP
	{
		//��ȡ��ǰͨ��
		int channel = Params1.toInt();

		int chk = 0;
		int SrcAdrr = StationID.toInt();
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 0x04;//֡����
		bytes[2] = 0x8f;//֡����
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];
		bytes[5] = channel;//ͨ��
		chk += bytes[5];
		bytes[6] = chk & 0xff;//У��λ �Ͱ�λ
		bytes[7] = (chk >> 8) & 0xff;//�߰�λ
		bytes[8] = 0xdd;
		::send(Socket, (char *)bytes, 9, 0);
		break;
	}
	case 1609:
		//��APN
	{
		int chk = 0;
		int SrcAdrr = StationID.toInt();
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 0x03;//֡����
		bytes[2] = 0x96;//֡����
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];
		bytes[5] = chk & 0xff;//У��λ �Ͱ�λ
		bytes[6] = (chk >> 8) & 0xff;//�߰�λ
		bytes[7] = 0xdd;
		::send(Socket, (char *)bytes, 8, 0);
		break;
	}
	case 1610:
		//дAPN
	{
		int chk = 0;
		int SrcAdrr = StationID.toInt();
		QString strAPN = Params1;
		QByteArray data = strAPN.toLatin1();
		int nCount = data.count();
		if (nCount > 16)
		{
		}
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 3 + nCount;
		bytes[2] = 0x95;
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];

		for (int i = 0; i < nCount; i++)
		{
			bytes[5 + i] = data.at(i);
			chk += bytes[5 + i];
		}
		bytes[5 + nCount] = chk & 0xff;
		bytes[6 + nCount] = (chk >> 8) & 0xff;
		bytes[7 + nCount] = 0xdd;
		::send(Socket, (char *)bytes, 8 + nCount, 0);
		break;
	}
	case 1611:
		//�����ϴ�ʱ����
	{
		int nTime = Params1.toInt();
		int chk = 0;
		int SrcAdrr = StationID.toInt();
		BYTE bytes[1024] = { 0 };
		bytes[0] = 0xaa;
		bytes[1] = 0x05;//֡����
		bytes[2] = 0xb9;//֡����
		chk += bytes[2];
		bytes[3] = SrcAdrr & 0xff;//Դ��ַ
		chk += bytes[3];
		bytes[4] = (SrcAdrr >> 8) & 0xff;
		chk += bytes[4];
		bytes[5] = nTime & 0xff;
		chk += bytes[5];
		bytes[6] = (nTime >> 8) & 0xff;
		chk += bytes[6];
		bytes[7] = chk & 0xff;//У��λ �Ͱ�λ
		bytes[8] = (chk >> 8) & 0xff;//�߰�λ
		bytes[9] = 0xdd;
		::send(Socket, (char *)bytes, 10, 0);
		break;
	}
	default:
		break;
	}
}
