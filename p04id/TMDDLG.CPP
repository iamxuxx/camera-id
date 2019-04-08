// TmdDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Tmd.h"
#include "TmdDlg.h"
#include "okapi32.h"
#include "sub.h"
#include <io.h>

#include "math.h"
#include <stdio.h>
#include <vector>
#include <istream>
#include <fstream> 
#include <string>
using namespace std;
  #include   <algorithm>
  #include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#define THRT 30
#define THR2 THRT*2
#define THR4 THR2*THR2

#pragma comment(lib, "OKAPI32")
#pragma comment(lib, "Sub")
//struct SH3 {long k,unsigned short g;float v;};
//struct SH4 {unsigned short i,j,g;float v;};	
//struct SH6 {unsigned short i,j,g1,g2;float v1,v2;};	
//struct RCRD {BYTE no,rng;SH4 s;};
struct ELPS{
	float	a,		//a轴
	  		b,		//b轴
			bt,		//a与x轴夹角
			ag,		//起点角
			me;		//均方差
	FXY		c;		//圆心
};	

HANDLE hBrd;
PNT *pnt,*pnu,BLACK={0,0,0},WHITE={255,255,255},RED={0,0,255};
	BYTE *bg=(BYTE *)new BYTE[HW];
BYTE * b0,gray;
RECT rct0;
FXYZ dt[6][25];

FXYZ stdd3(float *a,int n) ;
void Sort7z(FXYZ arr[],long cnt,BYTE flg);
void CTgt2Q(float id,FXYZ *dt1,float *Q);
float tsFx(float a,FXYZ t1,FXYZ c0);
float tsFind(float a,float b,FXYZ t1,FXYZ c0);
ELPS fitElp(FXYZ *xy,long n);
void genXX(FXYZ *xy,long n,float *XX);
FXYZ pdt2vct(FXYZ in,float &r);
float mods(FXYZ in){float r;r=sqrt(in.x*in.x+in.y*in.y+in.z*in.z);return r;};
void dsply();
FXYZ guiy1(FXYZ i,float &r){FXYZ f;r=sqrt(i.x*i.x+i.y*i.y+i.z*i.z);f.x=i.x/r;f.y=i.y/r;f.z=i.z/r;return f;}
float angle(FXYZ fi,FXYZ fj){
	FXYZ f0,f1;
	f0=guiy(fi);f1=guiy(fj);
	return acos(fi.x*fj.x+fi.y*fj.y+fi.z*fj.z);
}

vector <FXYZ> F1;

/////////////////////////////////////////////////////////////////////////////
// CTmdDlg dialog

CTmdDlg::CTmdDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTmdDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTmdDlg)
	m_id = 0.0f;
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTmdDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTmdDlg)
	DDX_Control(pDX, IDC_COMBO2, m_dot);
	DDX_Control(pDX, IDC_COMBO1, m_cmb);
	DDX_Control(pDX, IDC_LIST1, m_lst);
	DDX_Text(pDX, IDC_EDIT1, m_id);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTmdDlg, CDialog)
	//{{AFX_MSG_MAP(CTmdDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_BN_CLICKED(IDC_BUTTON2, OnButton2)
	ON_BN_CLICKED(IDC_BUTTON3, OnButton3)
	ON_BN_CLICKED(IDC_BUTTON4, OnButton4)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_WM_RBUTTONUP()
	ON_EN_CHANGE(IDC_EDIT1, OnChangeEdit1)
	ON_WM_LBUTTONDBLCLK()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTmdDlg message handlers

BOOL CTmdDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	SetWindowPos(NULL,0,0,1280,800,NULL);	
	long lIndex=-2030,l;
	char st[80];
	FILE *fp;
	hBrd= okOpenBoard(&lIndex);
	RECT rct;
	SetRect(&rct,0,0,0,0);
	okSetTargetRect(hBrd,BUFFER,&rct);
	SetRect(&rct,2,40,2+W2,40+HEIGHT);
//	SetRect(&rct,0,0,W2,HEIGHT);

	okSetTargetRect(hBrd,SCREEN,&rct);
//	l=okLoadImageFile(hBrd,"BMP\\a.bmp",0,BUFFER,0,1);
	l=okLoadImageFile(hBrd,"BMP\\sl0.bmp",0,BUFFER,0,1);
//	l=okLoadImageFile(hBrd,"JPG\\a0.jpg",0,BUFFER,0,1);
	sprintf(st,"l=%d",l);m_lst.AddString(st);
	b0=(BYTE *)okGetBufferAddr(hBrd,0);
	pnt=(PNT *)okGetBufferAddr(hBrd,0);
//	pnu=(PNT *)okGetBufferAddr(hBrd,1);
	CreateDirectory("DAT\\",NULL);
	CreateDirectory("JPG\\",NULL);
	m_cmb.SetCurSel(0);m_dot.SetCurSel(0);
	if(!(fp=fopen("DAT\\id.dat","rb"))){m_id=1427.5;UpdateData(false);}
	else{fread(&m_id,sizeof(float),1,fp);UpdateData(false);}

//	okConvertRect(hBrd,SCREEN,0,BUFFER,0,1);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTmdDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CTmdDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTmdDlg::OnButton1() 
{


m_lst.ResetContent();
return;

	long isum,jsum;
	int i0,j0,k1,wk,kcnt;
	FXYZ fi,*b1;
	FILE *fp;
	long k,l,n;
	int i,j,k0;
	char st[80];
	BLOCKINFO blk;
	long sum;


/*	BYTE *bf=(BYTE *)new BYTE [HEIGHT*W2*3];
	FillMemory(&blk,sizeof(blk),0);
	blk.iBitCount=24;
	blk.iFormType=FORM_RGB888;
	blk.iHeight=HEIGHT;
	blk.iWidth=W2;
	blk.lpBits = bf;
//		l=okLoadImageFile(hBrd,"BMP\\sl0.bmp",0,BUFFER,0,1);sprintf(st,"l=%d",l);m_lst.AddString(st);

	for(k=0;k<3;k++){
		sprintf(st,"BMP\\sl%d.bmp",k);
		l=okLoadImageFile(hBrd,st,0,BUFFER,0,1);sprintf(st,"l=%d",l);m_lst.AddString(st);
		for(i=0;i<HEIGHT;i++){
			CopyMemory(bf+W2*3*i,b0+WIDTH*3*i,W2*3);
		}
		sprintf(st,"BMP\\sl%d_0.bmp",k);
		okSaveImageFile(hBrd,st,100,TARGET(&blk),0,1);

		for(i=0;i<HEIGHT;i++){
			CopyMemory(bf+W2*3*i,b0+WIDTH*3*i+W2*3,W2*3);
		}
		sprintf(st,"BMP\\sl%d_1.bmp",k);
		okSaveImageFile(hBrd,st,100,TARGET(&blk),0,1);
	}
	delete [] bf;
return;*/

/*	BYTE *bf=(BYTE *)new BYTE [HEIGHT*W2];
	FillMemory(&blk,sizeof(blk),0);
	blk.iBitCount=8;
	blk.iFormType=FORM_GRAY8;
	blk.iHeight=HEIGHT;
	blk.iWidth=W2;
	blk.lpBits = bf;
	for(k0=0;k0<4;k0++){
		sprintf(st,"BMP\\a%d.bmp",k0);
		okLoadImageFile(hBrd,st,0,BUFFER,0,1);
		for(k=0;k<HEIGHT*W2;k++){bf[k]=(pnt[k].b*114+pnt[k].g*587+pnt[k].r*299)/1000;}
		l=okConvertRect(hBrd,SCREEN,0,TARGET(&blk),0,1);
		sprintf(st,"JPG\\a%d.jpg",k0);	
		okSaveImageFile(hBrd,st,0,TARGET(&blk),0,1);		
	}
	delete [] bf;
return;*/



//	sprintf(st,"JPG\\a%d.jpg",m_cmb.GetCurSel());
//	okLoadImageFile(hBrd,st,0,BUFFER,0,1);
//	dsply();

/*
if(!(fp=fopen("DAT\\dt.dat","rb")))return;
fread(dt,12,150,fp);fclose(fp);
for(k=0;k<6;k++){
m_lst.AddString("");
for(k0=0;k0<25;k0++){
	fi=dt[k][k0];
sprintf(st,"%02d %9.3f %9.3f %9.3f",k0,fi.x,fi.y,fi.z);m_lst.AddString(st);
}
}*/

return;
	for(k=0;k<3;k++)
	{
		for(k0=0;k0<2;k0++){
			sprintf(st,"BMP\\F%d_%d.bmp",k,k0);//m_lst.AddString(st);
			l=okLoadImageFile(hBrd,st,0,BUFFER,0,1);
			sprintf(st,"l=%d",l);m_lst.AddString(st);
			for(l=0;l<HEIGHT*W2;l++){b0[l]=(b0[l*3+0]*114+b0[l*3+1]*587+b0[l*3+2]*299)/1000;}
			sum=0;
			for(l=0;l<HEIGHT*W2;l++)sum+=b0[l];
			sum/=(HEIGHT*W2);sum*=1.2;
			for(i=0;i<HEIGHT;i++)for(j=0;j<350;j++)b0[i*W2+j]=sum;
			for(i=0;i<HEIGHT;i++)for(j=W2-150;j<W2;j++)b0[i*W2+j]=sum;

			BYTE *bf=(BYTE *)new BYTE[H2*W4];
			BYTE *bw=(BYTE *)new BYTE[H2*W4];

			for(i=0;i<H2;i++)CopyMemory(&bf[i*W4],&b0[i*W4],W4);
			im2bw(bf,bw,H2,W4);
			for(i=0;i<H2;i++)CopyMemory(&b0[i*W4],&bw[i*W4],W4);

			for(i=0;i<H2;i++)CopyMemory(&bf[i*W4],&b0[(i+H2)*W4],W4);
			im2bw(bf,bw,H2,W4);
			for(i=0;i<H2;i++)CopyMemory(&b0[(i+H2)*W4],&bw[i*W4],W4);
			for(i=0;i<H2;i++)CopyMemory(&bf[i*W4],&b0[(i+H2*2)*W4],W4);
			im2bw(bf,bw,H2,W4);
			for(i=0;i<H2;i++)CopyMemory(&b0[(i+H2*2)*W4],&bw[i*W4],W4);
			for(i=0;i<H2;i++)CopyMemory(&bf[i*W4],&b0[(i+H2*3)*W4],W4);
			im2bw(bf,bw,H2,W4);
			for(i=0;i<H2;i++)CopyMemory(&b0[(i+H2*3)*W4],&bw[i*W4],W4);

			delete [] bf,bw;

//			okConvertRect(hBrd,SCREEN,0,TARGET(&blk),0,1);Sleep(1000);

//			sprintf(st,"JPG\\F%d_%d.jpg",k,k0);m_lst.AddString(st);
			sprintf(st,"JPG\\a%d.jpg",k*2+k0);m_lst.AddString(st);
			l=okSaveImageFile(hBrd,st,70,TARGET(&blk),0,1);
			sprintf(st,"l=%d",l);m_lst.AddString(st);



		}
	}

//=========================================


	if(!(fp=fopen("DAT\\dt0.dat","rb")))
	{
	fp=fopen("DAT\\dt0.dat","wb");
	for(k=0;k<6;k++)
	{
		
			m_lst.AddString("");m_lst.AddString("========================");
			sprintf(st,"JPG\\a%d.jpg",k);
			m_lst.AddString(st);
			l=okLoadImageFile(hBrd,st,0,BUFFER,0,1);
			for(n=0;n<HEIGHT*W2;n++)if(b0[n]>127)b0[n]=255;else b0[n]=0;

			int *bg5=(int *)new int[HEIGHT*W2];
			FillMemory(bg5,sizeof(int)*HEIGHT*W2,0);
			for(n=0;n<HEIGHT*W2;n++)if(b0[n]>0)bg5[n]=1;

			wk=BWLabel(bg5,HEIGHT,W2);
			sprintf(st,"wk=%d",wk);m_lst.AddString(st);
			kcnt=0;
			for(k1=1;k1<=wk;k1++){		
			sum=isum=jsum=0;
			for(i=0;i<HEIGHT;i++)for(j=0;j<W2;j++){
				if(bg5[i*W2+j]==k1)
				{
					sum++;
					isum+=i;jsum+=j;
				}
			}
			if(sum>5){
				fi.x=float(jsum)/float(sum);
				fi.y=float(isum)/float(sum);
				fi.z=sum;
				fwrite(&fi,sizeof(FXYZ),1,fp);
				sprintf(st,"%02d %3d | %9.3f %9.3f %9.3f | %03d",k1,sum,fi.x,fi.y,fi.z,kcnt++);m_lst.AddString(st);
				i0=fi.y;j0=fi.x;
				for(i=-2;i<=2;i++)for(j=-2;j<=2;j++)b0[(i+i0)*W2+j+j0]=128;
			}
		}
		delete [] bg5;		
		okConvertRect(hBrd,SCREEN,0,BUFFER,0,1);			
		
	
	}
	fclose(fp);
	return;
	}
//===========================================
	float af,x,y,r;
	FXYZ dt0[6*25];FXYZ dt1[25]; 
	fread(dt0,sizeof(FXYZ),6*25,fp);fclose(fp);
	for(k=0;k<6;k++){CopyMemory(dt1,&dt0[k*25],sizeof(FXYZ)*25);
	Sort7z(dt1,25,1);
	for(i=0;i<24;i++)
	{
		x=dt1[i].x-dt1[24].x;y=dt1[i].y-dt1[24].y;r=sqrt(x*x+y*y);
		af=asin(y/r);if(x<0)af=pi-af;dt1[i].z=af;

	}
	dt1[24].z=0;

	for(i=23;i>=0;i--)
	{
		dt1[i].z-=dt1[0].z;
		if(dt1[i].z<0)dt1[i].z+=2*pi;
	}
	Sort7z(dt1,24,0);
//	m_lst.AddString("");for(i=0;i<25;i++){sprintf(st,"%d %02d %9.3f %9.3f %9.4f",k,i,dt1[i].x,dt1[i].y,dt1[i].z*180/pi);m_lst.AddString(st);}

	sprintf(st,"DAT\\dt1_%d.dat",k);
	fp=fopen(st,"wb");fwrite(dt1,sizeof(FXYZ),25,fp);fclose(fp);

	}
	ELPS ep[6];
//===========================================
	for(k=0;k<6;k++){
		sprintf(st,"DAT\\dt1_%d.dat",k);
		if(!(fp=fopen(st,"rb")))return;
		fread(dt1,sizeof(FXYZ),25,fp);fclose(fp);
		m_lst.AddString("");for(i=0;i<25;i++){sprintf(st," %d %02d %9.3f %9.3f %9.4f",k,i,dt1[i].x,dt1[i].y,dt1[i].z*180/pi);m_lst.AddString(st);}
//		sprintf(st,"JPG\\a%d.jpg",k);okLoadImageFile(hBrd,st,0,BUFFER,0,1);for(wk=0;wk<25;wk++){i0=dt1[wk].y;j0=dt1[wk].x;for(i=-4;i<=4;i++)for(j=-4;j<=4;j++)b0[(i+i0)*W2+j+j0]=128;okConvertRect(hBrd,SCREEN,0,BUFFER,0,1);Sleep(20);}okConvertRect(hBrd,SCREEN,0,BUFFER,0,1);Sleep(1000);
		ep[k]=fitElp(dt1,24);sprintf(st,"%d %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f",k,ep[k].a,ep[k].b,ep[k].c.x,ep[k].c.y,ep[k].bt*180/pi,ep[k].ag*180/pi);m_lst.AddString(st);
	}
	fp=fopen("DAT\\ep.dat","wb");fwrite(ep,sizeof(ELPS),6,fp);fclose(fp);
//===========================================


//===========================================
	m_lst.AddString("here");
	m_lst.AddString("Finished");
	sprintf(st,"HEIGHT=%4d W2=%4d",HEIGHT,W2);m_lst.AddString(st);
}
FXYZ pdt2vct(FXYZ in,float &r)
{
	CTmdDlg *pDlg = (CTmdDlg *)(AfxGetApp()->GetMainWnd());
	FXYZ o;
	float x,y,z;
	x=in.x-W4;y=in.y-H2;z=pDlg->m_id;r=sqrt(x*x+y*y+z*z);
	o.x=x/r;o.y=y/r;o.z=z/r;
	return o;
}

//		LOGFONT lfLogFont={32, 24, 0, 0, 100, 0, 0, 0, 0, 1, 2, 1, 26, "Arial"};
//		LOGFONT lfLogFont={64, 48, 0, 0, 400, 0, 0, 0, 0, 1, 2, 1, 26, "Arial"};
//		SETTEXTMODE	textmode={0x000000,0xffffff,0,0,0};//°×é?
//		SETTEXTMODE	textmode={0x000000,0x0000ff,0,0,0};//oìé?
//		RECT rect0={30,20,0,0};
//		rect0.top=100;
//		sprintf(st,"|¤b=%7.2f mm",ep.b-RADIUS);
//		okSetTextTo(hBrd,TARGET(&blk),&rect0,&lfLogFont,&textmode,st,20);

void CTmdDlg::OnButton2() 
{
	char st[80];
	int i,j,k0,k,i0,j0;
	FILE *fp;
	FXYZ fi,fj,t,tt[4];
	ELPS ep[6];
	float af,bt,gm,x,y,xp,yp,la,lb,La,Lb;
		LOGFONT lfLogFont={32, 24, 0, 0, 100, 0, 0, 0, 0, 1, 2, 1, 26, "Arial"};
//		LOGFONT lfLogFont={64, 48, 0, 0, 400, 0, 0, 0, 0, 1, 2, 1, 26, "Arial"};
		SETTEXTMODE	textmode={0x000000,0xffffff,0,0,0};//°×é?
//		SETTEXTMODE	textmode={0x000000,0x0000ff,0,0,0};//oìé?
//	RECT rect0={30,20,0,0};
//	okSetTextTo(hBrd,BUFFER,&rect0,&lfLogFont,&textmode,st,20);
	if(!(fp=fopen("DAT\\dt1.dat","rb"))){m_lst.AddString("data not found!");return;}
	m_lst.AddString("here");
	fread(dt,12,100,fp);fclose(fp);
	for(k=0;k<4;k++)ep[k]=fitElp(dt[k],24);
	for(k=0;k<2;k++){
		m_lst.AddString("");
		fi=dt[k*2][24];fj=dt[k*2+1][24];
		fi=pdt2vct(fi,la);fj=pdt2vct(fj,lb);
		La=la*RADIUS/ep[k].a;
		Lb=lb*RADIUS/ep[k].a;
		sprintf(st,"24 %9.6f %9.6f %9.6f |%9.6f %9.6f %9.6f",fi.x,fi.y,fi.z,fj.x,fj.y,fj.z);m_lst.AddString(st);
		sprintf(st,"lL %9.3f %9.3f %9.3f %9.3f",la,lb,La,Lb);m_lst.AddString(st);
		fi.x*=La;fi.y*=La;fi.z*=La;tt[k*2]=fi;
		fj.x*=Lb;fj.y*=Lb;fj.z*=Lb;tt[k*2+1]=fj;
		t=sbstrct(fi,fj);
		sprintf(st,"24 %9.3f %9.3f %9.3f |%9.3f %9.3f %9.3f",fi.x,fi.y,fi.z,fj.x,fj.y,fj.z);m_lst.AddString(st);
		sprintf(st," t %9.3f %9.3f %9.3f %9.3f",t.x,t.y,t.z,mods(t));m_lst.AddString(st);

		for(k0=0;k0<25;k0++){
		fi=dt[k*2][k0];fj=dt[k*2+1][k0];
		sprintf(st,"%02d %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f",k0,fi.x,fi.y,fi.z,fj.x,fj.y,fj.z);m_lst.AddString(st);	
		}
	}

	t=sbstrct(tt[0],tt[2]);sprintf(st," %9.3f %9.3f %9.3f %9.3f",t.x,t.y,t.z,mods(t));m_lst.AddString(st);
	t=sbstrct(tt[1],tt[3]);sprintf(st," %9.3f %9.3f %9.3f %9.3f",t.x,t.y,t.z,mods(t));m_lst.AddString(st);

		m_lst.AddString("=======================");


		m_lst.AddString("here");
//		for(k0=0;k0<25;k0++){fi=dt[k][k0];sprintf(st,"%02d %9.3f %9.3f %9.3f",k0,fi.x,fi.y,fi.z);m_lst.AddString(st);}
		
//		sprintf(st,"%d %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f",k,ep[k].a,ep[k].b,ep[k].c.x,ep[k].c.y,ep[k].bt*180/pi,ep[k].ag*180/pi);m_lst.AddString(st);
	

/*	k0=m_cmb.GetCurSel();
	sprintf(st,"JPG\\sl%d.jpg",k0);
	okLoadImageFile(hBrd,st,0,BUFFER,0,1);
	for(i=0;i<rct0.top ;i++)for(j=0;j<W2;j++)b0[i*W2+j]=gray;
	for(i=rct0.bottom;i<HEIGHT ;i++)for(j=0;j<W2;j++)b0[i*W2+j]=gray;
	for(j=0;j<rct0.left ;j++)for(i=0;i<HEIGHT;i++)b0[i*W2+j]=gray;	
	for(j=rct0.right;j<W2 ;j++)for(i=0;i<HEIGHT;i++)b0[i*W2+j]=gray;	
	okConvertRect(hBrd,SCREEN,0,BUFFER,0,1);
	sprintf(st,"JPG\\s%d.jpg",k0);
	okSaveImageFile(hBrd,st,70,BUFFER,0,1);
	k0++;k0%=6;
	m_cmb.SetCurSel(k0);*/

}
/*
void CTmdDlg::OnButton2() 
{
	int i,i0,j,j0,k0,k1;
	long l,n,k;
	char st[80];
	FILE *fp;
	ELPS ep[6];
	float a,r,af,bt,gm,t0[3],la,lb,Q[16],A[9],Qinv[16],U[16];
	FXYZ du[6][25],fi,t[6],tt[3],fj,fk;
//===========================================
	m_lst.ResetContent();
	if(!(fp=fopen("DAT\\ep.dat","rb")))return;
	fread(ep,sizeof(ELPS),6,fp);fclose(fp);
//	for(k=0;k<6;k++){sprintf(st,"%d %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f",k,ep[k].a,ep[k].b,ep[k].c.x,ep[k].c.y,ep[k].bt*180/pi,ep[k].ag*180/pi);m_lst.AddString(st);}
//	a=(RADIUS/ep[2].a-RADIUS/ep[0].a)*m_id;sprintf(st,"a=%9.3f",a);m_lst.AddString(st);a=(RADIUS/ep[4].a-RADIUS/ep[2].a)*m_id;sprintf(st,"a=%9.3f",a);m_lst.AddString(st);a=(RADIUS/ep[3].a-RADIUS/ep[1].a)*m_id;sprintf(st,"a=%9.3f",a);m_lst.AddString(st);a=(RADIUS/ep[5].a-RADIUS/ep[3].a)*m_id;sprintf(st,"a=%9.3f",a);m_lst.AddString(st);

	for(k=0;k<6;k++){
		fi.x=ep[k].c.x;fi.y=ep[k].c.y;
		fi=pdt2vct(fi,r);
		a=RADIUS*r/ep[k].a;
//		sprintf(st,"%9.6f %9.6f %9.6f %9.3f %9.3f",fi.x,fi.y,fi.z,r,a);m_lst.AddString(st);
		t[k].x=a*fi.x;t[k].y=a*fi.y;t[k].z=a*fi.z;
//		sprintf(st," %9.3f %9.3f %9.3f",t[k].x,t[k].y,t[k].z);m_lst.AddString(st);
	}
//	fi.x=fi.y=fi.z=3;fj.x=fj.y=fj.z=4;fk=sbstrct(fi,fj);sprintf(st,"%9.3f %9.3f %9.3f ",fk.x,fk.y,fk.z);m_lst.AddString(st);

//	fi=sbstrct(t[1],t[0]);sprintf(st,"%9.3f %9.3f %9.3f",fi.x,fi.y,fi.z);m_lst.AddString(st);
//	fi=sbstrct(t[3],t[2]);sprintf(st,"%9.3f %9.3f %9.3f",fi.x,fi.y,fi.z);m_lst.AddString(st);
//	fi=sbstrct(t[5],t[4]);sprintf(st,"%9.3f %9.3f %9.3f",fi.x,fi.y,fi.z);m_lst.AddString(st);
	tt[0]=sbstrct(t[1],t[0]);
	tt[1]=sbstrct(t[3],t[2]);
	tt[2]=sbstrct(t[5],t[4]);
	t0[0]=a=mods(tt[0]);//sprintf(st,"%9.3f",a);m_lst.AddString(st);
	t0[1]=a=mods(tt[1]);//sprintf(st,"%9.3f",a);m_lst.AddString(st);
	t0[2]=a=mods(tt[2]);//sprintf(st,"%9.3f",a);m_lst.AddString(st);

	fp=fopen("DAT\\t.dat","wb");fwrite(t,sizeof(FXYZ),6,fp);fclose(fp);
	fp=fopen("DAT\\tt.dat","wb");fwrite(tt,sizeof(FXYZ),3,fp);fclose(fp);

	for(k=0;k<6;k++){sprintf(st,"DAT\\dt1_%d.dat",k);if(!(fp=fopen(st,"rb")))return;fread(du[k],12,25,fp);fclose(fp);}
//	for(k=0;k<6;k++){sprintf(st,"JPG\\a%d.jpg",k);m_lst.AddString("");l=okLoadImageFile(hBrd,st,0,BUFFER,0,1);for(n=0;n<HEIGHT*W2;n++)if(b0[n]>127)b0[n]=255;else b0[n]=0;sprintf(st,"%d %9.3f %9.3f",k,ep[k].c.x,ep[k].c.y);m_lst.AddString(st);i0=ep[k].c.y;j0=ep[k].c.x;for(i=-3;i<=3;i++)for(j=-3;j<=3;j++)b0[(i0+i)*W2+j0+j]=127;okConvertRect(hBrd,SCREEN,0,BUFFER,0,1);Sleep(1000);for(k0=0;k0<25;k0++){fi=du[k][k0];sprintf(st,"%02d %9.3f %9.3f %9.3f",k0,fi.x,fi.y,fi.z);m_lst.AddString(st);}}
	for(k=0;k<6;k+=2)
	{
//		sprintf(st," %9.3f %9.3f %9.3f|%9.3f %9.3f %9.3f",t[k].x,t[k].y,t[k].z,t[k+1].x,t[k+1].y,t[k+1].z);m_lst.AddString(st);
//		sprintf(st," %9.3f %9.3f %9.3f",tt[k/2].x,tt[k/2].y,tt[k/2].z);m_lst.AddString(st);
		fk=tt[k/2];fk=guiy(fk);
//		sprintf(st," %9.6f %9.6f %9.6f t0=%9.3f",fk.x,fk.y,fk.z,t0[k/2]);m_lst.AddString(st);
//		fk=pdt2vct(tt[k/2],r);
//		sprintf(st," %9.6f %9.6f %9.6f %9.3f",fk.x,fk.y,fk.z,r);m_lst.AddString(st);
//		m_lst.AddString("");
		for(k0=0;k0<25;k0++)
		{
			fi=du[k][k0];fj=du[k+1][k0];
			fi=pdt2vct(fi,r);fj=pdt2vct(fj,r);
//			sprintf(st,"%9.3f %9.3f %9.3f|%9.3f %9.3f %9.3f",fi.x,fi.y,fi.z,fj.x,fj.y,fj.z);m_lst.AddString(st);
//			sprintf(st,"%9.6f %9.6f %9.6f|%9.6f %9.6f %9.6f",fi.x,fi.y,fi.z,fj.x,fj.y,fj.z);m_lst.AddString(st);
			gm=acos(fi.x*fj.x+fi.y*fj.y+fi.z*fj.z);
			fi=du[k][k0];
			fi=pdt2vct(fi,r);
			af=acos(fi.x*fk.x+fi.y*fk.y+fi.z*fk.z);
			bt=pi-af-gm;
//			sprintf(st,"%02d af=%9.4f bt=%9.4f %9.4f",k0,af*180/pi,bt*180/pi,gm*180/pi);m_lst.AddString(st);
			la=t0[k/2]/sin(gm)*sin(bt);
			lb=t0[k/2]/sin(gm)*sin(af);
//			sprintf(st,"%02d %9.4f %9.4f",k0,la,lb);m_lst.AddString(st);
			fi.x*=la;fi.y*=la;fi.z*=la;
			fj.x*=lb;fj.y*=lb;fj.z*=lb;
//			sprintf(st,"%9.3f %9.3f %9.3f|%9.3f %9.3f %9.3f",fi.x,fi.y,fi.z,fj.x,fj.y,fj.z);m_lst.AddString(st);
			du[k][k0]=fi;du[k+1][k0]=fj;
		}
	}

//==================================================
	for(k=0;k<6;k++){
//		m_lst.AddString("");for(k0=0;k0<25;k0++){fi=du[k][k0];sprintf(st,"%02d %9.3f %9.3f %9.3f",k0,fi.x,fi.y,fi.z);m_lst.AddString(st);}
		cld2eig(du[k],25,Q,A);
//		Q[3]=t[k].x;Q[7]=t[k].y;Q[11]=t[k].z;
		sprintf(st,"t %9.3f %9.3f %9.3f",t[k].x,t[k].y,t[k].z);m_lst.AddString(st);
		m_lst.AddString("Q:");for(i=0;i<4;i++){sprintf(st,"%9.6f %9.6f %9.6f %9.3f",Q[i*4+0],Q[i*4+1],Q[i*4+2],Q[i*4+3]);m_lst.AddString(st);}
		m_lst.AddString("A:");sprintf(st,"%9.3f %9.3f %9.3f",A[0],A[4],A[8]);m_lst.AddString(st);
		sprintf(st,"DAT\\Q%d.dat",k);
		fopen(st,"wb");fwrite(Q,64,1,fp);fclose(fp);
	}

	for(k=0;k<6;k++){
//		m_lst.AddString("");for(k0=0;k0<25;k0++){fi=du[k][k0];sprintf(st,"%02d %9.3f %9.3f %9.3f",k0,fi.x,fi.y,fi.z);m_lst.AddString(st);}
		m_lst.AddString("");
		for(k0=0;k0<24;k0++){
		fi=sbstrct(du[k][24],du[k][k0]);fk=fj=sbstrct(du[k][24],du[k][(k0+6)%24]);fi=guiy(fi);fj=guiy(fj);
		af=acos(fi.x*fj.x+fi.y*fj.y+fi.z*fj.z);a=mods(fk);
		sprintf(st,"af %9.4f r=%9.3f",af*180/pi,a);m_lst.AddString(st);
		}
	}


		if(!(fp=fopen("DAT\\Q0.dat","rb")))return;fread(Q,64,1,fp);fclose(fp);
		if(!(fp=fopen("DAT\\Q1.dat","rb")))return;fread(Qinv,64,1,fp);fclose(fp);inv4(Qinv);
		mmtrx4(Q,Qinv,U);
		fp=fopen("DAT\\Q01.dat","wb");fwrite(U,64,1,fp);fclose(fp);

		if(!(fp=fopen("DAT\\Q2.dat","rb")))return;fread(Q,64,1,fp);fclose(fp);
		if(!(fp=fopen("DAT\\Q3.dat","rb")))return;fread(Qinv,64,1,fp);fclose(fp);inv4(Qinv);
		mmtrx4(Q,Qinv,U);
		fp=fopen("DAT\\Q23.dat","wb");fwrite(U,64,1,fp);fclose(fp);

		if(!(fp=fopen("DAT\\Q4.dat","rb")))return;fread(Q,64,1,fp);fclose(fp);
		if(!(fp=fopen("DAT\\Q5.dat","rb")))return;fread(Qinv,64,1,fp);fclose(fp);inv4(Qinv);
		mmtrx4(Q,Qinv,U);
		fp=fopen("DAT\\Q45.dat","wb");fwrite(U,64,1,fp);fclose(fp);

//==================================================
	m_lst.AddString("");
	m_lst.AddString("Finished");
}
*/

void CTmdDlg::OnButton3() 
{
	char st[80];
	long l,k;
	int i,j,k0,i0,j0;
	FILE *fp;
	FXYZ tt[3],dt[6][25],fi,fj,fk;
	ELPS ep[6];
	float r,s,af,bt,gm,la,lb,La,Lb,t;
	if(!(fp=fopen("DAT\\tt.dat","rb")))return;
	fread(tt,12,3,fp);fclose(fp);
//	for(k0=0;k0<3;k0++){sprintf(st,"%9.3f %9.3f %9.3f",tt[k0].x,tt[k0].y,tt[k0].z);m_lst.AddString(st);}
	if(!(fp=fopen("DAT\\dt.dat","rb")))return;
	fread(dt,12,150,fp);fclose(fp);
//	for(k=0;k<6;k++){m_lst.AddString("");for(k0=0;k0<25;k0++){sprintf(st,"%9.3f %9.3f %9.3f",dt[k][k0].x,dt[k][k0].y,dt[k][k0].z);m_lst.AddString(st);}}


//===========================================
//	for(k=0;k<6;k++){ep[k]=fitElp(dt[k],24);sprintf(st,"%d %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f",k,ep[k].a,ep[k].b,ep[k].c.x,ep[k].c.y,ep[k].bt*180/pi,ep[k].ag*180/pi);m_lst.AddString(st);}
//	fp=fopen("DAT\\ep.dat","wb");fwrite(ep,sizeof(ELPS),6,fp);fclose(fp);

	if(!(fp=fopen("DAT\\ep.dat","rb")))return;
	fread(ep,sizeof(ELPS),6,fp);fclose(fp);
//	fk=pdt2vct(tt[0],r);
//sprintf(st,"tt %9.3f %9.3f %9.3f",tt[0].x,tt[0].y,tt[0].z);m_lst.AddString(st);

	for(k=0;k<3;k++){
		fi=dt[k*2][24];//sprintf(st,"fi %9.3f %9.3f %9.3f",fi.x,fi.y,fi.z);m_lst.AddString(st);  
		fi=pdt2vct(fi,la);
		La=la*RADIUS/ep[k*2].a;
		fj=dt[k*2+1][24];//sprintf(st,"fj %9.3f %9.3f %9.3f",fj.x,fj.y,fj.z);m_lst.AddString(st);
		fj=pdt2vct(fj,lb);
		Lb=lb*RADIUS/ep[k*2+1].a;

		sprintf(st,"l =%9.3f %9.3f | L =%9.3f %9.3f ",la,lb,La,Lb);m_lst.AddString(st);
		fi=dt[k*2][24];fi=pdt2vct(fi,r);fi.x*=La;fi.y*=La;fi.z*=La;
		fj=dt[k*2+1][24];fj=pdt2vct(fj,r);fj.x*=Lb;fj.y*=Lb;fj.z*=Lb;
		fk=sbstrct(fi,fj);

//sprintf(st,"fk %9.3f %9.3f %9.3f",fk.x,fk.y,fk.z);m_lst.AddString(st);
		
		t=mods(fk);fk=guiy(fk);
		m_lst.AddString("");
		for(k0=0;k0<25;k0++){
			fi=dt[k*2][k0];fi=pdt2vct(fi,r);
			fj=dt[k*2+1][k0];fj=pdt2vct(fj,s);
//			sprintf(st,"%02d %9.6f %9.6f %9.6f %9.3f",k0,fi.x,fi.y,fi.z,r);m_lst.AddString(st);
//			sprintf(st,"%02d %9.6f %9.6f %9.6f %9.3f",k0,fj.x,fj.y,fj.z,s);m_lst.AddString(st);
			gm=acos(fi.x*fj.x+fi.y*fj.y+fi.z*fj.z);
			af=acos(fi.x*fk.x+fi.y*fk.y+fi.z*fk.z);
			bt=pi-af-gm;
//			sprintf(st,"%02d %9.4f %9.4f %9.4f ",k0,af*180/pi,bt*180/pi,gm*180/pi);m_lst.AddString(st);	
			La=t/sin(gm)*sin(bt);
			Lb=t/sin(gm)*sin(af);
			fi=dt[k*2][k0];fi=pdt2vct(fi,r);fi.x*=La;fi.y*=La;fi.z*=La;
			fj=dt[k*2+1][k0];fj=pdt2vct(fj,s);fj.x*=Lb;fj.y*=Lb;fj.z*=Lb;
			sprintf(st,"%02d %9.3f %9.3f %9.3f|%9.3f %9.3f %9.3f ",k0,fi.x,fi.y,fi.z,fj.x,fj.y,fj.z);m_lst.AddString(st);
			dt[k*2][k0]=fi;dt[k*2+1][k0]=fj;
		}
	}
	fp=fopen("DAT\\dt3D.dat","wb");fwrite(dt,12,150,fp);fclose(fp);

	m_lst.AddString("");m_lst.AddString("finished");
}

void CTmdDlg::OnButton4() 
{
	// TODO: Add your control notification handler code here
	char st[80];
	int k,k0,i,i0,j,j0;
	FXYZ dt[6][25],fi,v0,v1,v2;
	float Q[16],A[9],af;
	FILE *fp;
	FABCD S;
	S.A=S.B=0;S.C=-1;S.D=m_id;
	if(!(fp=fopen("DAT\\dt3D.dat","rb")))return;
	fread(dt,12,150,fp);fclose(fp);
	for(k=0;k<6;k++){
		sprintf(st,"JPG\\sl%d.jpg",k);okLoadImageFile(hBrd,st,0,BUFFER,0,1);
		m_lst.AddString("");
		for(k0=0;k0<25;k0++){
			fi=dt[k][k0];
			sprintf(st,"%02d %9.3f %9.3f %9.3f",k0,fi.x,fi.y,fi.z);m_lst.AddString(st);
			fi=intsct(fi,S);
			i0=fi.y+H2;j0=fi.x+W4;
			for(i=-2;i<=2;i++)for(j=-2;j<=2;j++)b0[(i0+i)*W2+j0+j]=0;
//			dsply();Sleep(100);			
		}
//		dsply();Sleep(1000);
		for(k0=0;k0<24;k0++){
		v0=sbstrct(dt[k][k0],dt[k][24]);v0=guiy(v0);
		v1=sbstrct(dt[k][(k0+6)%24],dt[k][24]);v1=guiy(v1);
//		sprintf(st," %9.3f %9.3f %9.3f",v0.x,v0.y,v0.z);m_lst.AddString(st);
//		sprintf(st," %9.3f %9.3f %9.3f",v1.x,v1.y,v1.z);m_lst.AddString(st);
		af=acos(v0.x*v1.x+v0.y*v1.y+v0.z*v1.z);
		sprintf(st,"k0=%02d  af %9.4f",k0,af*180/pi);m_lst.AddString(st);
		}

		cld2eig(dt[k],25,Q,A);
		m_lst.AddString("Q:");for(i=0;i<4;i++){sprintf(st,"%9.6f %9.6f %9.6f %9.3f",Q[i*4+0],Q[i*4+1],Q[i*4+2],Q[i*4+3]);m_lst.AddString(st);}
		m_lst.AddString("A:");sprintf(st,"%9.3f %9.3f %9.3f",A[0],A[4],A[8]);m_lst.AddString(st);

	}
		
	

	m_lst.AddString("");m_lst.AddString("finished");
}

void CTmdDlg::OnButton5() 
{
	// TODO: Add your control notification handler code here

	m_lst.AddString("");
	m_lst.AddString("Finished");
}


void CTmdDlg::OnButton6() 
{
	// TODO: Add your control notification handler code here
	char st[80];
	int k1,k0,k,i0,i,j0,j;
	FILE *fp;
	FXYZ dt[4][25],fi,fj,fk,tt;
		FXYZ v0,v1,v2;
	if(!(fp=fopen("DAT\\dt1.dat","rb")))return;
	fread(dt,12,100,fp);fclose(fp);
//	for(k=0;k<4;k++){m_lst.AddString("");for(k0=0;k0<25;k0++){fi=dt[k][k0];sprintf(st,"%02d %9.3f %9.3f %9.3f ",k0,fi.x,fi.y,fi.z);m_lst.AddString(st);}}
	for(k=0;k<4;k++){sprintf(st,"JPG\\a%d.jpg",k);okLoadImageFile(hBrd,st,0,BUFFER,0,1);for(k0=0;k0<25;k0++){i0=dt[k][k0].y;j0=dt[k][k0].x;for(i=-3;i<=3;i++)for(j=-3;j<=3;j++)b0[(i0+i)*W2+j0+j]=0;dsply();Sleep(100);}dsply();Sleep(1000);}
	ELPS ep[4];
//===========================================
//	for(k=0;k<4;k++){ep[k]=fitElp(dt[k],24);sprintf(st,"%d %9.3f %9.3f %9.3f %9.3f %9.3f %9.3f",k,ep[k].a,ep[k].b,ep[k].c.x,ep[k].c.y,ep[k].bt*180/pi,ep[k].ag*180/pi);m_lst.AddString(st);}fp=fopen("DAT\\ep.dat","wb");fwrite(ep,sizeof(ELPS),4,fp);fclose(fp);
	if(!(fp=fopen("DAT\\ep.dat","rb")))return;fread(ep,sizeof(ELPS),4,fp);fclose(fp);
	float la,La,lb,Lb,lc,Lc,x,y,z,r,af,bt,gm,Q[4][16],Qinv[16],U[16],A[9],xp,yp;
	FXYZ c[4],C[4],D[4];
	for(k=0;k<4;k++){fi.x=ep[k].c.x-W4;fi.y=ep[k].c.y-H2;fi.z=m_id;c[k]=fi;}
	for(k=0;k<4;k++){
		fi=guiy1(c[k],la);
		La=la*RADIUS/ep[k].a;
		fj.x=La*fi.x;
		fj.y=La*fi.y;
		fj.z=La*fi.z;C[k]=fj;
//		sprintf(st,"   C %9.3f %9.3f %9.3f %9.3f ",fj.x,fj.y,fj.z,La);m_lst.AddString(st);
//		sprintf(st,"c[k] %9.3f %9.3f %9.3f %9.3f ",c[k].x,c[k].y,c[k].z,0);m_lst.AddString(st);
		fj=dt[k][0];fj.z=m_id;
		fj=sbstrct(c[k],fj);
//		sprintf(st,"  fj %9.3f %9.3f %9.3f %9.3f ",fj.x,fj.y,fj.z,0);m_lst.AddString(st);
		fi=guiy(c[k]);fj=guiy(fj);
//		sprintf(st,"  fi %9.3f %9.3f %9.3f %9.3f ",fi.x,fi.y,fi.z,0);m_lst.AddString(st);
//		sprintf(st,"  fj %9.3f %9.3f %9.3f %9.3f ",fj.x,fj.y,fj.z,0);m_lst.AddString(st);
//		ag=angle(fi,fj);sprintf(st,"ag=%9.5f",ag*180/pi);m_lst.AddString(st);
		v0=fj;v2=fi;
//		sprintf(st,"  v2 %9.6f %9.6f %9.6f %9.6f ",v2.x,v2.y,v2.z,0);m_lst.AddString(st);
		v1=normal(v2,v0);
		v2=normal(v0,v1);
//		sprintf(st,"  v2 %9.6f %9.6f %9.6f %9.6f ",v2.x,v2.y,v2.z,0);m_lst.AddString(st);
//		for(k0=0;k0<25;k0++){fj=dt[k][k0];sprintf(st,"   C %9.3f %9.3f %9.3f %9.3f ",fj.x,fj.y,fj.z,0);m_lst.AddString(st);}
		Q[k][0]=v0.x;Q[k][1]=v1.x;Q[k][2]=v2.x;Q[k][3]=C[k].x;
		Q[k][4]=v0.y;Q[k][5]=v1.y;Q[k][6]=v2.y;Q[k][7]=C[k].y;
		Q[k][8]=v0.z;Q[k][9]=v1.z;Q[k][10]=v2.z;Q[k][11]=C[k].z;
		Q[k][12]=Q[k][13]=Q[k][14]=0;Q[k][15]=1;
//		m_lst.AddString("Q[k]:");for(i=0;i<4;i++){sprintf(st,"%9.6f %9.6f %9.6f %9.3f",Q[k][i*4+0],Q[k][i*4+1],Q[k][i*4+2],Q[k][i*4+3]);m_lst.AddString(st);}
	}

	fp=fopen("DAT\\Q4.dat","wb");fwrite(Q,64,4,fp);fclose(fp);

		fj=sbstrct(C[0],C[2]);
		fi=guiy1(fj,r);
		sprintf(st," id %9.3f %9.3f %9.3f %9.3f",fj.x,fj.y,fj.z,r);m_lst.AddString(st);
		fj=sbstrct(C[1],C[3]);
		fi=guiy1(fj,r);
		sprintf(st," id %9.3f %9.3f %9.3f %9.3f",fj.x,fj.y,fj.z,r);m_lst.AddString(st);	

		fj=sbstrct(C[0],C[1]);
		fi=guiy1(fj,r);
		sprintf(st," fj %9.3f %9.3f %9.3f %9.3f",fj.x,fj.y,fj.z,r);m_lst.AddString(st);
		fj=sbstrct(C[2],C[3]);
		fi=guiy1(fj,r);tt=fj;
		sprintf(st," fj %9.3f %9.3f %9.3f %9.3f",fj.x,fj.y,fj.z,r);m_lst.AddString(st);
		fp=fopen("DAT\\tt.dat","wb");fwrite(&fj,12,1,fp);fclose(fp);
	
	tt=guiy1(tt,lc);

		m_lst.AddString("");
		for(k0=0;k0<25;k0++){
			fi=dt[0][k0];fj=dt[1][k0];
			fi=pdt2vct(fi,la);fj=pdt2vct(fj,lb);
			gm=acos(fi.x*fj.x+fi.y*fj.y+fi.z*fj.z);
			af=acos(fi.x*tt.x+fi.y*tt.y+fi.z*tt.z);
			bt=pi-af-gm;
//			sprintf(st,"%02d %9.4f %9.4f %9.4f ",k0,af*180/pi,bt*180/pi,gm*180/pi);m_lst.AddString(st);
			La=sin(bt)*lc/sin(gm);
			fi.x=La*fi.x;
			fi.y=La*fi.y;
			fi.z=La*fi.z;dt[0][k0]=fi;

			Lb=sin(af)*lc/sin(gm);
			fj.x=La*fj.x;
			fj.y=La*fj.y;
			fj.z=La*fj.z;dt[1][k0]=fj;
		}

		m_lst.AddString("");
		for(k0=0;k0<25;k0++){
			fi=dt[2][k0];fj=dt[3][k0];
			fi=pdt2vct(fi,la);fj=pdt2vct(fj,lb);
			gm=acos(fi.x*fj.x+fi.y*fj.y+fi.z*fj.z);
			af=acos(fi.x*tt.x+fi.y*tt.y+fi.z*tt.z);
			bt=pi-af-gm;
//			sprintf(st,"%02d %9.4f %9.4f %9.4f ",k0,af*180/pi,bt*180/pi,gm*180/pi);m_lst.AddString(st);

			La=sin(bt)*lc/sin(gm);
			fi.x=La*fi.x;
			fi.y=La*fi.y;
			fi.z=La*fi.z;dt[2][k0]=fi;

			Lb=sin(af)*lc/sin(gm);
			fj.x=La*fj.x;
			fj.y=La*fj.y;
			fj.z=La*fj.z;dt[3][k0]=fj;
		
		}
		for(k=0;k<4;k++){
//			m_lst.AddString("");
			for(k0=0;k0<25;k0++){
				fi=dt[k][k0];
//				sprintf(st,"%02d %9.3f %9.3f %9.3f",k0,fi.x,fi.y,fi.z);m_lst.AddString(st);
			}
		}
	
		fp=fopen("DAT\\dt3D.dat","wb");	fwrite(dt,12,100,fp);fclose(fp);	
	
		for(k=0;k<4;k++){
			fi=dt[k][24];
//			sprintf(st,"c %9.3f %9.3f%9.3f",fi.x,fi.y,fi.z);m_lst.AddString(st);
			sprintf(st,"Q[%d]:",k);
			m_lst.AddString(st);for(i=0;i<4;i++){sprintf(st,"%9.6f %9.6f %9.6f %9.3f",Q[k][i*4+0],Q[k][i*4+1],Q[k][i*4+2],Q[k][i*4+3]);m_lst.AddString(st);}
		}

/*		for(k=0;k<4;k++){
			fi=dt[k][24];
			sprintf(st,"c %9.3f %9.3f%9.3f",fi.x,fi.y,fi.z);m_lst.AddString(st);
			cld2eig(dt[k],24,Q[k],A);
			m_lst.AddString("Q[k]:");for(i=0;i<4;i++){sprintf(st,"%9.6f %9.6f %9.6f %9.3f",Q[k][i*4+0],Q[k][i*4+1],Q[k][i*4+2],Q[k][i*4+3]);m_lst.AddString(st);}
			m_lst.AddString("A:");sprintf(st,"%9.3f %9.3f %9.3f",A[0],A[4],A[8]);m_lst.AddString(st);
			m_lst.AddString("");
		}*/
//=============================

	RECT rect0={30,20,0,0};
		LOGFONT lfLogFont={32, 24, 0, 0, 100, 0, 0, 0, 0, 1, 2, 1, 26, "Arial"};
//		LOGFONT lfLogFont={64, 48, 0, 0, 400, 0, 0, 0, 0, 1, 2, 1, 26, "Arial"};
		SETTEXTMODE	textmode={0x000000,0xffffff,0,0,0};//°×é?
//		SETTEXTMODE	textmode={0x000000,0x0000ff,0,0,0};//oìé?
//	for(k=0;k<1;k++)
	for(k=0;k<4;k++)
	{
		sprintf(st,"JPG\\a%d.jpg",k);
		okLoadImageFile(hBrd,st,0,BUFFER,0,1);//dsply();Sleep(1000);
		for(k0=0;k0<360;k0++){
			af=k0*180/pi;
			y=ep[k].b*sin(af);
			x=ep[k].a*cos(af);
			xp=x*cos(ep[k].bt)+y*sin(ep[k].bt);
			yp=-x*sin(ep[k].bt)+y*cos(ep[k].bt);
			fi.x=xp+ep[k].c.x;
			fi.y=yp+ep[k].c.y;
		
//			sprintf(st,"%03d %9.3f %9.3f",k0,fi.x,fi.y);m_lst.AddString(st);
			i=fi.y;j=fi.x;
			if(j>0&&i>0)b0[i*W2+j]=255;

		rect0.top=20;
		sprintf(st,"bt=%9.4f",ep[k].bt*180/pi);
		okSetTextTo(hBrd,BUFFER,&rect0,&lfLogFont,&textmode,st,20);
		for(i=0;i<HEIGHT;i+=2)b0[i*W2+W4]=255;
		for(j=0;j<W2;j+=2)b0[H2*W2+j]=255;
		for(k1=-320;k1<320;k1++){
			fi.x=ep[k].c.x+k1*cos(ep[k].bt);
			fi.y=ep[k].c.y+k1*sin(ep[k].bt);
			i=fi.y;j=fi.x;
			b0[i*W2+j]=0;
		}
		for(k1=-320;k1<320;k1++){
			fi.x=ep[k].c.x+k1*cos(ep[k].bt+pi/2);
			fi.y=ep[k].c.y+k1*sin(ep[k].bt+pi/2);
			i=fi.y;j=fi.x;
			b0[i*W2+j]=255;
		}
		}
		sprintf(st,"JPG\\b%d.jpg",k);
		okSaveImageFile(hBrd,st,70,BUFFER,0,1);

//		sprintf(st,"C %9.3f %9.3f %9.3f",C[k].x,C[k].y,C[k].z);m_lst.AddString(st);

		m_lst.AddString("");
		v2=guiy1(C[k],r);
		sprintf(st,"v2 %9.6f %9.6f %9.6f %9.3f",v2.x,v2.y,v2.z,r);m_lst.AddString(st);
//		v1=sbstrct(C[k],D[k]);v1=guiy1(v1,r);
//		sprintf(st,"v1 %9.6f %9.6f %9.6f %9.3f",v2.x,v2.y,v2.z,r);m_lst.AddString(st);


		dsply();Sleep(1000);
	}

/*	m_lst.ResetContent();
	for(k=0;k<4;k++){
		m_lst.AddString("");
		for(k0=0;k0<25;k0++){
			fi=dt[k][k0];
			sprintf(st," %02d  %9.3f %9.3f %9.3f",k0,fi.x,fi.y,fi.z);m_lst.AddString(st); 
		}
	}
*/
	float tj[100];
	m_lst.ResetContent();
	int cnt=0;
	for(k=0;k<4;k++){
		m_lst.AddString("");
		for(k0=0;k0<25;k0++){
			k1=k0+12;k1%=24;
			fi=dt[k][k0];fj=dt[k][k1];
			fk=sbstrct(fj,fi);

			fk=guiy1(fk,lb);
			sprintf(st," %02d  %9.6f %9.6f %9.6f %9.3f",k0,fk.x,fk.y,fk.z,lb);m_lst.AddString(st); 
			if(k0<24)tj[cnt++]=lb;
		}
	}


	m_lst.AddString("");
	
	
fi=stdd3(tj,96) ;	
sprintf(st," %9.3f %9.6f %9.0f ",fi.x,fi.y,fi.z);m_lst.AddString(st);
	
	m_lst.AddString("");m_lst.AddString("");
	m_lst.AddString("");m_lst.AddString("");



//==============================
/*	CopyMemory(Qinv,Q[2],64);inv4(Qinv);
	mmtrx4(Q[0],Qinv,U);inv4(U);
	m_lst.AddString("U:");for(i=0;i<4;i++){sprintf(st,"%9.6f %9.6f %9.6f %9.3f",U[i*4+0],U[i*4+1],U[i*4+2],U[i*4+3]);m_lst.AddString(st);}
	CopyMemory(Qinv,Q[3],64);inv4(Qinv);
	mmtrx4(Q[1],Qinv,U);inv4(U);
	m_lst.AddString("U:");for(i=0;i<4;i++){sprintf(st,"%9.6f %9.6f %9.6f %9.3f",U[i*4+0],U[i*4+1],U[i*4+2],U[i*4+3]);m_lst.AddString(st);}


	CopyMemory(Qinv,Q[1],64);inv4(Qinv);
	mmtrx4(Q[0],Qinv,U);inv4(U);
	m_lst.AddString("U:");for(i=0;i<4;i++){sprintf(st,"%9.6f %9.6f %9.6f %9.3f",U[i*4+0],U[i*4+1],U[i*4+2],U[i*4+3]);m_lst.AddString(st);}
	CopyMemory(Qinv,Q[3],64);inv4(Qinv);
	mmtrx4(Q[2],Qinv,U);inv4(U);
	m_lst.AddString("U:");for(i=0;i<4;i++){sprintf(st,"%9.6f %9.6f %9.6f %9.3f",U[i*4+0],U[i*4+1],U[i*4+2],U[i*4+3]);m_lst.AddString(st);}
*/
	m_lst.AddString("here");
}

void CTmdDlg::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default

	CDialog::OnRButtonUp(nFlags, point);
}

FXYZ stdd3(float *a,int n) 
{
	FXYZ ret;
	float sum=0,ave,x,sg;
	int k;
	sum=0;for(k=0;k<n;k++)sum+=a[k];ave=sum/n;
	sum=0;for(k=0;k<n;k++){x=a[k]-ave;sum+=x*x;}
	sg=sqrt(sum/n);
	ret.x=ave;ret.y=sg;ret.z=n;
	return ret;
}
void Sort7z(FXYZ arr[],long cnt,BYTE flg){
	int smallNo;  
	int pass,j;     
	FXYZ temp; 
	if(flg==0){
	for (pass=0;pass<cnt-1;pass++){ 
		smallNo=pass; 
		for(j=pass+1;j<cnt;j++) 
			if(arr[j].z<arr[smallNo].z)smallNo=j; 
		if(smallNo!=pass){ 
			temp=arr[pass]; 
			arr[pass]=arr[smallNo]; 
			arr[smallNo]=temp; 
		} 
	} 
	}
	else{
	for (pass=0;pass<cnt-1;pass++){ 
		smallNo=pass; 
		for(j=pass+1;j<cnt;j++) 
			if(arr[j].z>arr[smallNo].z)smallNo=j; 
		if(smallNo!=pass){ 
			temp=arr[pass]; 
			arr[pass]=arr[smallNo]; 
			arr[smallNo]=temp; 
		} 
	} 
	}
}

void CTmdDlg::OnChangeEdit1() 
{
	UpdateData(true);	
}


void genXX(FXYZ *xy,long n,float *XX)
{
	long k;
	float xx,Xy,yy,x,y,z;
	FillMemory(XX,sizeof(float)*30,0);
	for(k=0;k<n;k++)	
	{
		if(xy[k].x==0.&&xy[k].y==0.)continue;
		{
		xx=xy[k].x*xy[k].x;
		Xy=xy[k].x*xy[k].y;
		yy=xy[k].y*xy[k].y;
		x=xy[k].x;
		y=xy[k].y;
		z=-xx;

		XX[0]+=yy*yy;
		XX[6]+=Xy*Xy;
		XX[12]+=xx;
		XX[18]+=yy;
		XX[24]+=1;

		XX[1]+=yy*Xy;
		XX[2]+=yy*x;
		XX[3]+=yy*y;
		XX[4]+=yy;

		XX[7]+=Xy*x;
		XX[8]+=Xy*y;
		XX[9]+=Xy;

		XX[13]+=Xy;
		XX[14]+=x;

		XX[19]+=y;

		XX[25]+=yy*z;
		XX[26]+=Xy*z;
		XX[27]+=x*z;
		XX[28]+=y*z;	
		XX[29]+=z;	
		}
	}

//对称
	XX[5]=XX[1];

	XX[10]=XX[2];
	XX[11]=XX[7];

	XX[15]=XX[3];
	XX[16]=XX[8];
	XX[17]=XX[13];

	XX[20]=XX[4];
	XX[21]=XX[9];
	XX[22]=XX[14];
	XX[23]=XX[19];
}

ELPS fitElp(FXYZ *xy,long n)
{
	FXYZ zr={0.,0.,0.};FXY p;
//	char st[80];
	ELPS e;
	float *XX=(float *)new float[30];
	long k,kcnt;
	float U,V,W,X,Y,Z,Q,C2,C4,xp,xm;
	float xc,yc,a,b,bt,A,B,C,sume,w0,w3,x,y,r;
	FXYZ xy0;
	int ret;
	CTmdDlg *pDlg = (CTmdDlg *)(AfxGetApp()->GetMainWnd());char st[80];
	xy0=xy[0];
	genXX(xy,n,XX);
//	for(i=0;i<6;i++){sprintf(st,"%16.3f %16.3f %16.3f %16.3f %16.3f",XX[i*5],XX[i*5+1],XX[i*5+2],XX[i*5+3],XX[i*5+4]);pDlg->m_lst.AddString(st);}
	ret=all_gauss(5,XX,&XX[25]);
	if(ret==0){/*AfxMessageBox("all_gauss fail!");*/FillMemory(&e,sizeof(ELPS),0);return e;}

//pDlg->m_lst.AddString("");	sprintf(st,"%16.3f %16.3f %16.3f %16.3f %16.3f",XX[25],XX[26],XX[27],XX[28],XX[29]);pDlg->m_lst.AddString(st);
//pDlg->m_lst.AddString("");

	yc=(2*XX[28]/XX[26]-XX[27])/(XX[26]-4*XX[25]/XX[26]);	//圆心
	xc=(-XX[26]*yc-XX[27])/2.;e.c.x=xc;e.c.y=yc;			//圆心

	Q=(1.-XX[25])/XX[26];

	C=Q+sqrt(Q*Q+1.);
	bt=atan(C);

	e.bt=bt;										//倾角

	U=1./(xc*xc+XX[25]*yc*yc+XX[26]*xc*yc-XX[29]);
	V=XX[25]*U;
	W=XX[26]*U;
	X=XX[27]*U;
	Y=XX[28]*U;
	Z=XX[29]*U-1;

//pDlg->m_lst.AddString("");
//	sprintf(st,"%15.12f %15.12f %15.12f",U,V,W);pDlg->m_lst.AddString(st);
//	sprintf(st,"%15.12f %15.12f %15.12f",X,Y,Z);pDlg->m_lst.AddString(st);
//pDlg->m_lst.AddString("");
	C2=C*C;
	C4=C2*C2;
	A=sqrt((U-C2*V)/(1.-C4));
	B=sqrt((V-C2*U)/(1.-C4));

	a=cos(bt)/A;e.a=a;										//横轴
	b=cos(bt)/B;e.b=b;										//竖轴


//==================================
//==================================
//(b(x-xc))^2+(a(y-yc))^2-(ab)^2=0  
//(a(y-yc))^2=(ab)^2-(b(x-xc))^2
//x=xc+sqrt(b^2-(y-yc)^2)*a/b

/*	sume=0.;kcnt=0;		//计算均方差
	for(k=0;k<n;k++)	
	{
		if(xy[k].x==0.&&xy[k].y==0.)continue;
		p.x=(xy[k].x-xc)*cos(bt)-(xy[k].y-yc)*sin(bt);
		p.y=(xy[k].x-xc)*sin(bt)+(xy[k].y-yc)*cos(bt);
		w0=(b+p.y)*(b-p.y);
		if(w0<0.)continue;
		xp=+sqrt(w0)*a/b;
		xm=-sqrt(w0)*a/b;	
		if(fabs(p.x-xp)<fabs(p.x-xm))xy[k].z=w3=p.x-xp;else xy[k].z=w3=p.x-xm;
		sume+=(w3*w3);kcnt++;		
	}
	sume/=kcnt;
	e.me=sqrt(sume);
	for(k=0;k<n;k++)if(fabs(xy[k].z)>1.*e.me)xy[k]=zr;	*/

	sume=0.;kcnt=0;
	for(k=0;k<n;k++)	
	{
		if(xy[k].x==0.&&xy[k].y==0.)continue;
		p.x=(xy[k].x-xc)*cos(bt)-(xy[k].y-yc)*sin(bt);
		p.y=(xy[k].x-xc)*sin(bt)+(xy[k].y-yc)*cos(bt);
		w0=p.x*p.x/(a*a)+p.y*p.y/(b*b)-1.;
		xy[k].z=w0=fabs(w0);
		sume+=w0;kcnt++;	
	}
	sume/=kcnt;
	e.me=sqrt(sume);
//	for(k=0;k<n;k++)if(fabs(xy[k].z)>0.06)xy[k]=zr;   //计算点到椭圆圆周的距离，并过滤****过滤参数1
	for(k=0;k<n;k++)if(fabs(xy[k].z)>0.03)xy[k]=zr;    //计算点到椭圆圆周的距离，并过滤****过滤参数1

//======================================	
	genXX(xy,n,XX);
//	for(i=0;i<6;i++){sprintf(st,"%16.3f %16.3f %16.3f %16.3f %16.3f",XX[i*5],XX[i*5+1],XX[i*5+2],XX[i*5+3],XX[i*5+4]);pDlg->m_lst.AddString(st);}
	ret=all_gauss(5,XX,&XX[25]);
	if(ret==0){/*AfxMessageBox("all_gauss fail!");*/FillMemory(&e,sizeof(ELPS),0);return e;}

//pDlg->m_lst.AddString("");	sprintf(st,"%16.3f %16.3f %16.3f %16.3f %16.3f",XX[25],XX[26],XX[27],XX[28],XX[29]);pDlg->m_lst.AddString(st);
//pDlg->m_lst.AddString("");

	yc=(2*XX[28]/XX[26]-XX[27])/(XX[26]-4*XX[25]/XX[26]);	//圆心
	xc=(-XX[26]*yc-XX[27])/2.;e.c.x=xc;e.c.y=yc;			//圆心
	
	Q=(1.-XX[25])/XX[26];
	C=Q+sqrt(Q*Q+1.);
	bt=atan(C);

 //	if(bt>pi/4.)bt-=pi/2.;

	e.bt=bt;										//倾角??????????

	U=1./(xc*xc+XX[25]*yc*yc+XX[26]*xc*yc-XX[29]);
	V=XX[25]*U;
	W=XX[26]*U;
	X=XX[27]*U;
	Y=XX[28]*U;
	Z=XX[29]*U-1;

//pDlg->m_lst.AddString("");
//	sprintf(st,"%15.12f %15.12f %15.12f",U,V,W);pDlg->m_lst.AddString(st);
//	sprintf(st,"%15.12f %15.12f %15.12f",X,Y,Z);pDlg->m_lst.AddString(st);
//pDlg->m_lst.AddString("");

	C2=C*C;
	C4=C2*C2;
	A=sqrt((U-C2*V)/(1.-C4));
	B=sqrt((V-C2*U)/(1.-C4));

	a=cos(bt)/A;e.a=a;										//横轴
	b=cos(bt)/B;e.b=b;										//竖轴
	if(a<b){e.a=b;e.b=a;e.bt-=pi/2;}
	
	x=xy0.x-e.c.x;y=xy0.y-e.c.y;r=sqrt(x*x+y*y);
	w0=asin(y/r);if(x<0)w0=pi-w0;
	e.ag=w0;
//	sprintf(st,"xy0 %9.3f %9.3f %9.4f°",xy0.x,xy0.y,w0*180/pi);pDlg->m_lst.AddString(st);
//	sprintf(st,"xyr %9.3f %9.3f %9.3f",x,y,r);pDlg->m_lst.AddString(st);
//	sprintf(st,"e.c %9.3f %9.3f",e.c.x,e.c.y);pDlg->m_lst.AddString(st);

//================================
	sume=0.;kcnt=0;		//计算均方差
	for(k=0;k<n;k++)	
	{
		if(xy[k].x==0.&&xy[k].y==0.)continue;
		p.x=(xy[k].x-xc)*cos(bt)-(xy[k].y-yc)*sin(bt);
		p.y=(xy[k].x-xc)*sin(bt)+(xy[k].y-yc)*cos(bt);
		w0=(b+p.y)*(b-p.y);
		if(w0<0.)continue;
		xp=+sqrt(w0)*a/b;
		xm=-sqrt(w0)*a/b;	
		if(fabs(p.x-xp)<fabs(p.x-xm))xy[k].z=w3=p.x-xp;else xy[k].z=w3=p.x-xm;
		sume+=(w3*w3);kcnt++;		
	}
	sume/=kcnt;
	e.me=sqrt(sume);
//	for(k=0;k<n;k++)if(fabs(xy[k].z)>1.*e.me)xy[k]=zr;	       //计算均方差，并过滤****过滤参数2

//	if(e.bt>1.)e.bt=fabs(e.bt-(pi/2.));
//	if(e.bt>pi/4.)e.bt=fabs(e.bt-(pi/2.));

	delete [] XX;

	return e;
}

float tsFx(float a,FXYZ t1,FXYZ c0)
{
	CTmdDlg *pDlg = (CTmdDlg *)(AfxGetApp()->GetMainWnd());//char st[80];
	float x,y,z,r;
	x=a*t1.x-c0.x;
	y=a*t1.y-c0.y;
	z=a*t1.z-c0.z;;
	r=sqrt(x*x+y*y+z*z);
//	sprintf(st,"xyz %9.3f %9.3f %9.3f %9.3f",x,y,z,r-RADIUS);pDlg->m_lst.AddString(st);
//	sprintf(st,"c0 %9.3f %9.3f %9.3f",c0.x,c0.y,c0.z);pDlg->m_lst.AddString(st);
//	sprintf(st,"t1 %9.6f %9.6f %9.6f",t1.x,t1.y,t1.z);pDlg->m_lst.AddString(st);
	return r-RADIUS;
}
float tsFind(float a,float b,FXYZ t1,FXYZ c0)
{
	CTmdDlg *pDlg = (CTmdDlg *)(AfxGetApp()->GetMainWnd());//char st[80];
	float lb,fa,fb,fm,m;
	m=(a+b)/2;
//	sprintf(st,"a=%9.3f b=%9.3f",a,b);pDlg->m_lst.AddString(st);
	fa=tsFx(a,t1,c0);
	fb=tsFx(b,t1,c0);
	if(fa*fb>0){pDlg->m_lst.AddString("fa*fb>0!");return 0;}
	fm=tsFx(m,t1,c0);
	while(fabs(a-b)>0.0001){
		m=(a+b)/2;
		if(fa*fm>0){fa=fm;a=m;}else{fb=fm;b=m;}
		fm=tsFx(m,t1,c0);
//		sprintf(st,"%9.3f %9.3f %9.3f|%9.3f %9.3f %9.3f",a,b,m,fa,fb,fm);pDlg->m_lst.AddString(st);
//		sprintf(st,"fa=%9.3f fb=%9.3f fm=%9.3f",fa,fb,fm);pDlg->m_lst.AddString(st);
	}
	lb=a;
	return lb;
}

void CTmdDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	char st[80];
	int i0,j0,i,j,k0,k1;;
	CPoint lt;
	float sum,isum,jsum;
	FXYZ fi;
	i0=point.y*3/2;j0=point.x*3/2;
//	sprintf(st,"i0=%4d j0=%4d",i0,j0);m_lst.AddString(st);
	lt.x=rct0.left =j0-THRT;rct0.right =j0+THRT;
	lt.y=rct0.top =i0-THRT;rct0.bottom =i0+THRT;	
//	sprintf(st,"%4d %4d %4d %4d",rct0.left,rct0.top,rct0.right,rct0.bottom);m_lst.AddString(st);
	FILE *fp;
	if(!(fp=fopen("DAT\\dt.dat","rb"))){m_lst.AddString("data not found!");return;}
	fread(dt,12,150,fp);fclose(fp);
	k0=m_cmb.GetCurSel();
	k1=m_dot.GetCurSel();
	BLOCKINFO blk;
	BYTE *bf=(BYTE *)new BYTE [THR4];
	BYTE *bw=(BYTE *)new BYTE [THR4];
	FillMemory(&blk,sizeof(blk),0);
	blk.iBitCount=8;
	blk.iFormType=FORM_GRAY8;
	blk.iHeight=THR2;
	blk.iWidth=THR2;
	blk.lpBits = bw;
	for(i=0;i<THR2;i++)for(j=0;j<THR2;j++){
		bf[i*THR2+j]=b0[(lt.y+i)*W2+lt.x+j];
	}
	im2bw(bf,bw,THR2,THR2);
	sum=isum=jsum=0;
	for(i=0;i<THR2;i++)for(j=0;j<THR2;j++){if(bw[i*THR2+j]>127){sum++;isum+=i;jsum+=j;}}
	isum/=sum;jsum/=sum;
	i0=isum;j0=jsum;
	for(i=-1;i<=1;i++)for(j=-1;j<=1;j++)bw[(i0+i)*THR2+j0+j]=0;

	RECT rct;SetRect(&rct,2,40,2+THR2,40+THR2);
	okSetTargetRect(hBrd,SCREEN,&rct);	
	okConvertRect(hBrd,SCREEN,0,TARGET(&blk),0,1);
	delete [] bf,bw;
	dt[k0][k1].x=lt.x+jsum;
	dt[k0][k1].y=lt.y+isum;
	fi=dt[k0][k1];
	sprintf(st,"%d %02d | %9.3f,%9.3f",k0,k1,fi.x,fi.y);m_lst.AddString(st);

	i0=fi.y;j0=fi.x;

	for(i=-2;i<=2;i++)for(j=-2;j<=2;j++)b0[(i+i0)*W2+j+j0]=0;
	dsply();

	k1=m_dot.GetCurSel();
	k1++;k1%=25;m_dot.SetCurSel(k1);
	if(k1==0){
		k0=m_cmb.GetCurSel();
		k0++;k0%=6;m_cmb.SetCurSel(k0);
	}
	fp=fopen("DAT\\dt.dat","wb");fwrite(dt,12,150,fp);fclose(fp);




	CDialog::OnLButtonDblClk(nFlags, point);
}
void dsply(){
	RECT rct;
//	SetRect(&rct,2,40,2+W2,40+HEIGHT);
	SetRect(&rct,0,0,W2,HEIGHT);
	okSetTargetRect(hBrd,SCREEN,&rct);
	okConvertRect(hBrd,SCREEN,0,BUFFER,0,1);
}