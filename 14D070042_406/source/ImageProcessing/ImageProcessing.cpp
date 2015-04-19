

/****** INCLUDING VARIOUS LIBRARIES RREQIURED FOR IMAGE PROCESSING AND XBEE CONTROL ******/

#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include<math.h>
#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include<windows.h>

using namespace cv;

using namespace std;

HANDLE hPort = CreateFile("COM3", GENERIC_WRITE|GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);   //FOR XBEE CONTROL THROUGH USB PORT COM3(IT MAY BE DIFFERENT, DEPENDING ON THE SYSTEM ON WHICH THE PROGRAM IS RUN)
DCB dcb;                 //FOR XBEE

int r[480][640],b[480][640],g[480][640];          //FOR STORING THE RGB VALUES OF A PICTURE OF SIZE 480X640
const int row=480;
const int col=640;


/************ XBEE RELATED FUNCTIONS FOR READING AND WRITING DATA *************/


bool writebyte (char* data)
{
    DWORD byteswritten;
    if (!GetCommState(hPort, &dcb))
    {
        cout<<"\n Serial port can't be opened";
        return false;
    }

    dcb.BaudRate = CBR_9600;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    if (!SetCommState(hPort, &dcb))
    return false;

    bool retVal = WriteFile(hPort, data, 1, &byteswritten, NULL);
    return retVal;
}

int readByte()
{
    int Val;
    BYTE byte;
    DWORD dwBytesTransferred;
    DWORD dwCommModemStatus;
    if(!GetCommState(hPort, &dcb))
        return 0;

    SetCommMask(hPort, EV_RXCHAR|EV_ERR);
    WaitCommEvent(hPort, &dwCommModemStatus, 0);
    if (dwCommModemStatus& EV_RXCHAR)
        ReadFile(hPort, &byte, 1, &dwBytesTransferred, 0);
    Val = byte;
    return Val;
}

/*********** FUNCTIONS FOR SRORING THE RGB VALUES OF A PICTURE AND THEN DOING IMAGE PROCESSING**********/

void Blue(Mat image)
{
  for(int i = row/3; i <2*row/3; i++)
    {
    for(int j = col/3; j < 2*col/3; j++)
    {
      b[i][j] = image.at<cv::Vec3b>(i,j)[0];
    }
    }
}
void Green(Mat image)
{
  int flag1=0;
  for(int i = row/3; i <2*row/3; i++)
    {
    for(int j = col/3; j < 2*col/3; j++)
    {
      g[i][j] = image.at<cv::Vec3b>(i,j)[1];
    }
    }
}
void Red(Mat image)
{
  int flag1=0;
  for(int i = row/3; i <2*row/3; i++)
    {
    for(int j = col/3; j < 2*col/3; j++)
    {
      r[i][j] = image.at<cv::Vec3b>(i,j)[2];
    }
    }
}

int ImgProcessing(Mat img)
{
  Red(img);
  Blue(img);
  Green(img);
  int flag=0;
  for(int i = row/4; i <3*row/4; i++)
    {
    for(int j = col/4; j < 3*col/4; j++)
    {
      if(r[i][j]>b[i][j]+25 && r[i][j]>g[i][j]+25)
        flag++;
    }
    }
  cout<<flag<<endl;
  if(flag>8000)
    return 1;
  else
    return 0;
}

/************MAIN PROGRAM *****************/


int main( int argc, const char** argv )          //THE ARGUEMENTS IN THE MAIN PROGRAM ARE USED FOR IMAGE CAPTURING PURPOSE
{
  CvCapture* capture = cvCaptureFromCAM( 2 );     //STARTING THE CAMERA NO. '2'
  if ( !capture )                                //CHECKING IF THE CAMERA TURNED ON OR NOT
  {
    fprintf( stderr, "ERROR: capture is NULL \n" );
    getchar();
    return -1;
  }
  char ask_if_capture=0x38;
  char k;
  char data;
  int i;
  while(1)
  {
    if (writebyte(&ask_if_capture))              // SENDING THE DATA TO ASK WHETHER TO CAPTURE IMAGE OR NOT USING ECHO METHOD
    k=readByte();
    if( k=='1' || k=='2' )
    {
        Mat frame = cvQueryFrame( capture );     //CAPTURING IMAGE
        Mat img = cvCreateImage(cvSize(480,640), IPL_DEPTH_16S, 2);
        img = frame;
        cvNamedWindow( "LIVE CAPTURED IMAGE", CV_WINDOW_AUTOSIZE );
        imshow( "LIVE CAPTURED IMAGE", img );
        i=ImgProcessing(img);
        cout<<i<<endl;
        // FOR GIVING APPROPRIATE SIGNALS TO THE BOT ACCORDING TO THE IMAGE PROCESSED IMAGE
        if(k=='1')
        {
            if(i==1)
            {
                data = 0x31;
                if (writebyte(&data)) cout<<"Data Sent for i";
                cout<<"\n Data Received::"<<readByte()<<endl;
            }
            else
            {
                data = 0x30;
                if (writebyte(&data)) cout<<"Data Sent for i";
                cout<<"\n Data Received::"<<readByte()<<endl;
            }
        }
        else if(k=='2')
        {
            if(i==1)
            {
                data = 0x32;
                if (writebyte(&data)) cout<<"Data Sent for j";
                cout<<"\n Data Received::"<<readByte()<<endl;
            }
            else
            {
                data = 0x33;
                if (writebyte(&data)) cout<<"Data Sent for j";
                cout<<"\n Data Received::"<<readByte()<<endl;
            }
            cv::waitKey(2000);
        }
    }
    else if(k==0)
    {
        continue;
    }
    cv::waitKey(500);                     // WAITING SO AS TO AVOID PROBLEM TO ANY SIGNAL DELAYS
  }
  cvReleaseCapture( &capture );           // SHUTTING OF THE CAMERA
}

