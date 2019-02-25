#include "setparamdlg.h"

SetParamDlg::SetParamDlg(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);
	setWindowFlags(Qt::WindowCloseButtonHint | Qt::FramelessWindowHint);
	ui.CloseBtn->setToolTipName(QString::fromLocal8Bit("关闭"));
	ui.MinBtn->setToolTipName(QString::fromLocal8Bit("最小化"));
	connect(ui.MinBtn, SIGNAL(clicked()), this, SLOT(slot_minWindow()));//最小化
	connect(ui.CloseBtn, SIGNAL(clicked()), this, SLOT(close()));//关闭窗体
	ui.ComboBox_channel->addItem("...");
	ui.ComboBox_channel->addItem("1");
	ui.ComboBox_channel->addItem("2");
	ui.ComboBox_channel->addItem("3");
	ui.ComboBox_channel->addItem("4");
	connect(ui.ComboBox_channel, SIGNAL(currentIndexChanged(int)), this, SLOT(OnChannelIndexChanged(int)));
	connect(ui.WOpenChannelBtn, SIGNAL(clicked()), this, SLOT(OnWOpenChannelBtnClick()));
	connect(ui.WCloseChannelBtn, SIGNAL(clicked()), this, SLOT(OnWCloseChannelBtnClick()));
	connect(ui.WIPBtn, SIGNAL(clicked()), this, SLOT(OnWIPBtnClick()));
	connect(ui.RIPBtn, SIGNAL(clicked()), this, SLOT(OnRIPBtnClick()));
	connect(ui.DownBtn, SIGNAL(clicked()), this, SLOT(OnDownBtnClick()));
	connect(ui.SendDataTimeBtn, SIGNAL(clicked()), this, SLOT(OnSendDataTimeBtnClick()));
	connect(ui.WTimeBtn, SIGNAL(clicked()), this, SLOT(OnWTimeBtnClick()));
	connect(ui.RTimeBtn, SIGNAL(clicked()), this, SLOT(OnRTimeBtnClick()));
	connect(ui.RAPNBtn, SIGNAL(clicked()), this, SLOT(OnRAPNBtnClick()));
	connect(ui.WAPNBtn, SIGNAL(clicked()), this, SLOT(OnWAPNBtnClick()));
	connect(ui.UpdateBtn, SIGNAL(clicked()), this, SLOT(onUpdateBtnClick()));
}

SetParamDlg::~SetParamDlg()
{

}

void SetParamDlg::closeEvent(QCloseEvent *event)
{
	QWidget::close();
	*isActive = false;
	delete this;
}

void SetParamDlg::slot_minWindow()
{
	this->showMinimized();
}

void SetParamDlg::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		m_Drag = true;
		m_DragPosition = event->globalPos() - this->pos();
		event->accept();
	}
}

void SetParamDlg::mouseMoveEvent(QMouseEvent *event)
{
	if (m_Drag && (event->buttons() && Qt::LeftButton)) {
		move(event->globalPos() - m_DragPosition);
		event->accept();
	}
}

void SetParamDlg::mouseReleaseEvent(QMouseEvent *event)
{
	m_Drag = false;
}
//通道状态
 void SetParamDlg:: OnChannelIndexChanged(int index)
 {
	 if (index == 0)
		 return;
 	ui.label_channel_status->setText(QString::fromLocal8Bit("读取数据中..."));
	cmmIndex = 1604;
 	int chk = 0;
 	int SrcAdrr = StationID.toInt();
 	BYTE bytes[1024] = { 0 };
 	bytes[0] = 0xaa;
 	bytes[1] = 0x04;//帧长度
 	bytes[2] = 0x9d;//帧命令
 	chk += bytes[2];
 	bytes[3] = SrcAdrr & 0xff;//源地址
 	chk += bytes[3];
 	bytes[4] = (SrcAdrr >> 8) & 0xff;
 	chk += bytes[4];
 	bytes[5] = index;
 	chk += bytes[5];
 	bytes[6] = chk & 0xff;//校验位 低八位
 	bytes[7] = (chk >> 8) & 0xff;//高八位
 	bytes[8] = 0xdd;
 	::send(Socket, (char *)bytes, 9, 0);
}

//打开通道
 void SetParamDlg::OnWOpenChannelBtnClick()
 {
	 cmmIndex = 1605;
	 //获取当前通道
	 int channel = ui.ComboBox_channel->currentIndex() ;
	 if (channel == 0)
		 return;
	 //打开通道
	 int chk = 0;
	 int SrcAdrr = StationID.toInt();
	 BYTE bytes[1024] = { 0 };
	 bytes[0] = 0xaa;
	 bytes[1] = 0x05;//帧长度
	 bytes[2] = 0x9c;//帧命令
	 chk += bytes[2];
	 bytes[3] = SrcAdrr & 0xff;//源地址
	 chk += bytes[3];
	 bytes[4] = (SrcAdrr >> 8) & 0xff;
	 chk += bytes[4];
	 bytes[5] = channel;
	 chk += bytes[5];
	 bytes[6] = 0x1;
	 chk += bytes[6];
	 bytes[7] = chk & 0xff;//校验位 低八位
	 bytes[8] = (chk >> 8) & 0xff;//高八位
	 bytes[9] = 0xdd;
	 ::send(Socket, (char *)bytes, 10, 0);
 }

//关闭通道
 void SetParamDlg::OnWCloseChannelBtnClick()
 {
	 cmmIndex = 1606;
	 //获取当前通道
	 int channel = ui.ComboBox_channel->currentIndex();
	 if (channel == 0)
		 return;
	 int chk = 0;
	 int SrcAdrr = StationID.toInt();
	 BYTE bytes[1024] = { 0 };
	 bytes[0] = 0xaa;
	 bytes[1] = 0x05;//帧长度
	 bytes[2] = 0x9c;//帧命令
	 chk += bytes[2];
	 bytes[3] = SrcAdrr & 0xff;//源地址
	 chk += bytes[3];
	 bytes[4] = (SrcAdrr >> 8) & 0xff;
	 chk += bytes[4];
	 bytes[5] = channel;
	 chk += bytes[5];
	 bytes[6] = 0x0;
	 chk += bytes[6];
	 bytes[7] = chk & 0xff;//校验位 低八位
	 bytes[8] = (chk >> 8) & 0xff;//高八位
	 bytes[9] = 0xdd;
	 ::send(Socket, (char *)bytes, 10, 0);
 }

//设置IP
 void SetParamDlg::OnWIPBtnClick()
 {
	 cmmIndex = 1607;
	 //获取当前通道
	 int channel = ui.ComboBox_channel->currentIndex();
	 if (channel==0)
		 return;
	 int ip1 = 0, ip2 = 0, ip3 = 0, ip4 = 0, port = 0;
	 ip1 = ui.DevicelineEdit_IP1->text().toInt();
	 ip2 = ui.DevicelineEdit_IP2->text().toInt();
	 ip3 = ui.DevicelineEdit_IP3->text().toInt();
	 ip4 = ui.DevicelineEdit_IP4->text().toInt();
	 port = ui.DevicelineEdit_Port->text().toInt();
	 int chk = 0;
	 int SrcAdrr = StationID.toInt();
	 BYTE bytes[1024] = { 0 };
	 bytes[0] = 0xaa;
	 bytes[1] = 0x10;//帧长度
	 bytes[2] = 0x8e;//帧命令
	 chk += bytes[2];
	 bytes[3] = SrcAdrr & 0xff;//源地址
	 chk += bytes[3];
	 bytes[4] = (SrcAdrr >> 8) & 0xff;
	 chk += bytes[4];
	 bytes[5] = channel;//通道
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
	 bytes[12] = chk & 0xff;//校验位 低八位
	 bytes[13] = (chk >> 8) & 0xff;//高八位
	 bytes[14] = 0xdd;
	 ::send(Socket, (char *)bytes, 15, 0);
 }

//读取IP
 void SetParamDlg::OnRIPBtnClick()
 {
	 cmmIndex = 1608;
	 //获取当前通道
	 int channel = ui.ComboBox_channel->currentIndex();
	 if (channel == 0)
		 return;
	 int chk = 0;
	 int SrcAdrr = StationID.toInt();
	 BYTE bytes[1024] = { 0 };
	 bytes[0] = 0xaa;
	 bytes[1] = 0x04;//帧长度
	 bytes[2] = 0x8f;//帧命令
	 chk += bytes[2];
	 bytes[3] = SrcAdrr & 0xff;//源地址
	 chk += bytes[3];
	 bytes[4] = (SrcAdrr >> 8) & 0xff;
	 chk += bytes[4];
	 bytes[5] = channel;//通道
	 chk += bytes[5];
	 bytes[6] = chk & 0xff;//校验位 低八位
	 bytes[7] = (chk >> 8) & 0xff;//高八位
	 bytes[8] = 0xdd;
	 ::send(Socket, (char *)bytes, 9, 0);
 }

//补抄数据
 void SetParamDlg:: OnDownBtnClick()
 {
	 cmmIndex = 1603;
	 QString yearB, monthB, dayB, hourB, minB, yearE, monthE, dayE, hourE, minE;
	 yearB = ui.BdateTimeEdit->sectionText(QDateTimeEdit::YearSection);
	 monthB = ui.BdateTimeEdit->sectionText(QDateTimeEdit::MonthSection);
	 dayB = ui.BdateTimeEdit->sectionText(QDateTimeEdit::DaySection);
	 hourB = ui.BdateTimeEdit->sectionText(QDateTimeEdit::HourSection);
	 minB = ui.BdateTimeEdit->sectionText(QDateTimeEdit::MinuteSection);

	 yearE = ui.EdateTimeEdit->sectionText(QDateTimeEdit::YearSection);
	 monthE = ui.EdateTimeEdit->sectionText(QDateTimeEdit::MonthSection);
	 dayE = ui.EdateTimeEdit->sectionText(QDateTimeEdit::DaySection);
	 hourE = ui.EdateTimeEdit->sectionText(QDateTimeEdit::HourSection);
	 minE = ui.EdateTimeEdit->sectionText(QDateTimeEdit::MinuteSection);
	 int chk = 0;
	 int SrcAdrr = StationID.toInt();
	 BYTE bytes[1024] = { 0 };
	 bytes[0] = 0xaa;
	 bytes[1] = 0x0d;//帧长度
	 bytes[2] = 0x94;//帧命令
	 chk += bytes[2];
	 bytes[3] = SrcAdrr & 0xff;//源地址
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
	 bytes[15] = chk & 0xff;//校验位 低八位
	 bytes[16] = (chk >> 8) & 0xff;//高八位
	 bytes[17] = 0xdd;
	 ::send(Socket, (char *)bytes, 18, 0);
 }

 //数据上传时间间隔
 void SetParamDlg:: OnSendDataTimeBtnClick()
 {
	 cmmIndex = 1611;
	 int nTime = (ui.lineEdit_SendDataTime->text().trimmed()).toInt();
	 int chk = 0;
	 int SrcAdrr = StationID.toInt();
	 BYTE bytes[1024] = { 0 };
	 bytes[0] = 0xaa;
	 bytes[1] = 0x05;//帧长度
	 bytes[2] = 0xb9;//帧命令
	 chk += bytes[2];
	 bytes[3] = SrcAdrr & 0xff;//源地址
	 chk += bytes[3];
	 bytes[4] = (SrcAdrr >> 8) & 0xff;
	 chk += bytes[4];
	 bytes[5] = nTime & 0xff;
	 chk += bytes[5];
	 bytes[6] = (nTime >> 8) & 0xff;
	 chk += bytes[6];
	 bytes[7] = chk & 0xff;//校验位 低八位
	 bytes[8] = (chk >> 8) & 0xff;//高八位
	 bytes[9] = 0xdd;
	 ::send(Socket, (char *)bytes, 10, 0);
 }

//设置时钟
 void SetParamDlg::OnWTimeBtnClick()
 {
	 cmmIndex = 1601;
	 //获取当前时钟
	 QDateTime nowtime = QDateTime::currentDateTime();
	 QString datetime = nowtime.toString("yyyy.MM.dd hh:mm:ss");
	 QString year, month, day, hour, min, sec;
	 year = nowtime.toString("yy");
	 month = nowtime.toString("MM");
	 day = nowtime.toString("dd");
	 hour = nowtime.toString("hh");
	 min = nowtime.toString("mm");
	 sec = nowtime.toString("ss");
	 //设置时钟
	 int chk = 0;
	 int SrcAdrr = StationID.toInt();
	 BYTE bytes[1024] = { 0 };
	 bytes[0] = 0xaa;
	 bytes[1] = 0x0a;//帧长度
	 bytes[2] = 0x81;//帧命令
	 chk += bytes[2];
	 bytes[3] = SrcAdrr & 0xff;//源地址
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
	 bytes[12] = chk & 0xff;//校验位 低八位
	 bytes[13] = (chk >> 8) & 0xff;//高八位
	 bytes[14] = 0xdd;
	 ::send(Socket, (char *)bytes, 15, 0);
 }

//读取时钟
 void SetParamDlg::OnRTimeBtnClick()
 {
	 cmmIndex = 1602;
	 int chk = 0;
	 int SrcAdrr = StationID.toInt();
	 BYTE bytes[1024] = { 0 };
	 bytes[0] = 0xaa;
	 bytes[1] = 0x03;//帧长度
	 bytes[2] = 0x82;//帧命令
	 chk += bytes[2];
	 bytes[3] = SrcAdrr & 0xff;//源地址
	 chk += bytes[3];
	 bytes[4] = (SrcAdrr >> 8) & 0xff;
	 chk += bytes[4];
	 bytes[5] = chk & 0xff;//校验位 低八位
	 bytes[6] = (chk >> 8) & 0xff;//高八位
	 bytes[7] = 0xdd;
	 ::send(Socket, (char *)bytes, 8, 0);
 }

//读APN
 void SetParamDlg::OnRAPNBtnClick()
 {
	 cmmIndex = 1609;
	 int chk = 0;
	 int SrcAdrr = StationID.toInt();
	 BYTE bytes[1024] = { 0 };
	 bytes[0] = 0xaa;
	 bytes[1] = 0x03;//帧长度
	 bytes[2] = 0x96;//帧命令
	 chk += bytes[2];
	 bytes[3] = SrcAdrr & 0xff;//源地址
	 chk += bytes[3];
	 bytes[4] = (SrcAdrr >> 8) & 0xff;
	 chk += bytes[4];
	 bytes[5] = chk & 0xff;//校验位 低八位
	 bytes[6] = (chk >> 8) & 0xff;//高八位
	 bytes[7] = 0xdd;
	 ::send(Socket, (char *)bytes, 8, 0);
 }

//写APN
 void SetParamDlg::OnWAPNBtnClick()
 {
	 cmmIndex = 1610;
	 int chk = 0;
	 int SrcAdrr = StationID.toInt();
	 QString strAPN = ui.lineEdit_APN->text().trimmed();
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
	 bytes[3] = SrcAdrr & 0xff;//源地址
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
 }

 //远程升级
void SetParamDlg::onUpdateBtnClick()
{

}

 void SetParamDlg::setValue(QStringList list)
 {
	 switch (cmmIndex)
	 {
	 case 1604://状态
	 {
		 if (list.count() < 1)
			 break;
		 QString Status;
		 Status = list.at(1).compare("1") == 0 ? QString::fromLocal8Bit("打开") : QString::fromLocal8Bit("关闭");
		 ui.label_channel_status->setText(Status);
		 break;
	 }
	 case 1608://IP加端口
	 {
		 if (list.count() < 2)
			 break;
		 QStringList ip = list.at(0).split(".");
		 if (ip.count() < 4)
			 break;
		 ui.DevicelineEdit_IP1->setText(ip.at(0));
		 ui.DevicelineEdit_IP2->setText(ip.at(1));
		 ui.DevicelineEdit_IP3->setText(ip.at(2));
		 ui.DevicelineEdit_IP4->setText(ip.at(3));
		 ui.DevicelineEdit_Port->setText(list.at(1));
		 break;
	 }
	 case 1609://APN
	 {
		 if (list.count() < 1)
			 break;
		 ui.lineEdit_APN->setText(list.at(0));
		 break;
	 }
	 case 1602://设备时间
	 {
		 if (list.count() < 1)
			 break;
		 QDateTime time = QDateTime::fromString(list.at(0), "yyyy-MM-dd hh:mm:ss");
		 ui.RdateTimeEdit->setDateTime(time);
		 break;
	 }
	 default:
	 {
		 if (list.count() < 1)
			 break;
		 QString r = list.at(0);
		 if (r.compare(QString::fromLocal8Bit("通信结束")) == 0)
		 {
			 QMessageBox::about(this, QString::fromLocal8Bit("设备状态"), QString::fromLocal8Bit("操作成功！"));
		 }
	 }
		 break;
	 }
 }


