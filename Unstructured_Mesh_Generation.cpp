#include <iostream>
#include <fstream>

void infile(int&, double[][3]);
void minmax(int&, int, double[][3], double[][3]);
void initialize(int&, int[][4], int[][4]);
void circum(int, int[][4], double[][3], double&, double&, double&);
bool search(int&, int, int, int[][4], int[][4], int[], double, double, double, double&, double&, double&, double[][3]);
bool find(ofstream&, int, int, int&, int[][4], int[], double, double, double, double[], double[][3]);
void insert(ofstream&, int, int, int[], int[][4], double[], int[][4], int&, int[][4], int[][4]);
void rebuild(ofstream&, int, int, int&, int[][4], int[][4], int[][4], int[][4], int[], double[], double[][3]);

using namespace std;

int main()
{
     int i, j, iadd, nadd, nnode, nelem, isrch, idel, ndel, ielem, curr_elem;
     int iniel[100][4], ieliel[100][4], msrch[100], ielidel[100];
     int inielnew[100][4], ielielnew[100][4];
     bool srchflag, findflag;
     double x0, y0, rcs, xc1, xc2;
     double dvtoli;
     double smin, s1, s2, s3;
     double x[100][3], xadd[100][3], dv[100], xe[100][3];

     ofstream write("c:\\ugfinal.txt");

     smin = 0.00000001;
     dvtoli = 0.00000001;
     x0 = 0.0;
     y0 = 0.0;
     s1 = 0.0;
     s2 = 0.0;
     s3 = 0.0;
     ndel = 0;

     infile(nadd, xadd);
     write<<"list of points to be added..."<<endl<<"nadd = "<<nadd<<endl;
     for(i = 1; i <= nadd; i++) write<<"xadd["<<i<<"]["<<1<<"] = "<<xadd[i][1]<<"    xadd["<<i<<"]["<<2<<"] = "<<xadd[i][2]<<endl;
     write<<endl;

     minmax(nnode,nadd,xadd,x);
     write<<"creating minmax box..."<<endl<<"nnode = "<<nnode<<endl;
     for(i = 1; i <= nnode; i++) write<<"x["<<i<<"]["<<1<<"] = "<<x[i][1]<<"    x["<<i<<"]["<<2<<"] = "<<x[i][2]<<endl;
     write<<endl;

     initialize(nelem,iniel,ieliel);
     write<<"connecting inital triangulation..."<<endl<<"nelem = "<<nelem<<endl;
     for(i = 1; i <= nelem; i++) write<<"iniel["<<i<<"]["<<1<<"] = "<<iniel[i][1]<<"    iniel["<<i<<"]["<<2<<"] = "<<iniel[i][2]<<"    iniel["<<i<<"]["<<3<<"] = "<<iniel[i][3]<<endl;
     for(i = 1; i <= nelem; i++) write<<"ieliel["<<i<<"]["<<1<<"] = "<<ieliel[i][1]<<"    ieliel["<<i<<"]["<<2<<"] = "<<ieliel[i][2]<<"    ieliel["<<i<<"]["<<3<<"] = "<<ieliel[i][3]<<endl;
     write<<endl;

     isrch = 1;
     for(iadd = 1; iadd <= nadd; iadd++)
     {
          ielem = iadd;
          x0 = xadd[iadd][1];
          y0 = xadd[iadd][2];

          write<<"  starting circum algorithm..."<<endl;
          for(i = 1; i <= nelem; i++)
          {
               circum(i,iniel,x,rcs,xc1,xc2);
               dv[i] = rcs;
               xe[i][1] = xc1;
               xe[i][2] = xc2;
               write<<"ielem = "<<i<<"  rcs = "<<rcs<<"  xc1 = "<<xc1<<"  xc2 = "<<xc2<<endl;
          }
          write<<endl;

          write<<"  starting search algorithm..."<<endl;
          for(i = 1; i <= nelem; i++) msrch[i] = 0;
          srchflag = search(ielem,nelem,isrch,ieliel,iniel,msrch,smin,x0,y0,s1,s2,s3,x);
          write<<"  srchflag = "<<srchflag<<"  elem found = "<<ielem<<endl<<endl;

          write<<"  starting find algorithm..."<<endl;
          findflag = find(write,ielem,nelem,ndel,ieliel,ielidel,dvtoli,x0,y0,dv,xe);
          write<<"  findflag = "<<findflag<<endl<<endl;
          write<<"ndel = "<<ndel<<endl;
          for(i = 1; i <= ndel; i++) write<<"i = "<<i<<"  ielidel = "<<ielidel[i]<<endl;
          write<<endl;

          write<<"  starting insert algorithm..."<<endl;
          nnode++;
          x[nnode][1] = x0;
          x[nnode][2] = y0;
          insert(write,nnode,ndel,ielidel,ieliel,dv,iniel,nadd,inielnew,ielielnew);
          write<<endl;

          write<<"  starting rebuild algorithm..."<<endl;
          rebuild(write,nadd,ndel,nelem,iniel,ieliel,inielnew,ielielnew,ielidel,dv,xe);
          for(idel = 1; idel <= ndel; idel++)
          {
               curr_elem = ielidel[idel];
               if(dv[curr_elem] < 0) dv[curr_elem] = -dv[curr_elem];
          }
          write<<endl;

          write<<"  showing final iteration..."<<endl;
          write<<"nelem = "<<nelem<<endl<<endl;
          for(i = 1; i <= nelem; i++)
          {
               write<<"ielem = "<<i<<endl;
               write<<"  iniel1 = "<<iniel[i][1]<<endl;
               write<<"  iniel2 = "<<iniel[i][2]<<endl;
               write<<"  iniel3 = "<<iniel[i][3]<<endl;
               write<<"  ieliel1 = "<<ieliel[i][1]<<endl;
               write<<"  ieliel2 = "<<ieliel[i][2]<<endl;
               write<<"  ieliel3 = "<<ieliel[i][3]<<endl;
          }
          write<<endl;

          isrch++;
     }

     write.close();
     return(0);
}

void infile(int& num_pts, double x[][3])
{
     int i;

     ifstream read("c:\\circle.bedge");

     read>>num_pts;
     for(i = 1; i <= num_pts; i++) read>>x[i][1]>>x[i][2];

     read.close();
     return;
}

void minmax(int& nnode, int nadd, double xadd[][3], double x[][3])
{
     int i;
     double dx, dy, xmin, xmax, ymin, ymax;

     nnode = 4;

     xmin = xadd[1][1];
     xmax = xadd[1][1];
     ymin = xadd[1][2];
     ymax = xadd[1][2];
     for(i = 2; i <= nadd; i++)
     {
          if(xadd[i][1] < xmin) xmin = xadd[i][1];
          if(xadd[i][1] > xmax) xmin = xadd[i][1];
          if(xadd[i][2] < ymin) ymin = xadd[i][2];
          if(xadd[i][2] > ymax) ymax = xadd[i][2];
     }

     dx = xmax - xmin;
     dy = ymax - ymin;

     x[1][1] = xmin - dx;
     x[1][2] = ymax + dy;
     x[2][1] = xmax + dx;
     x[2][2] = ymax + dy;
     x[3][1] = xmin - dx;
     x[3][2] = ymin - dy;
     x[4][1] = xmax + (1.1*dx);
     x[4][2] = ymin - (1.1*dy);

     return;
}

void initialize(int& nelem, int iniel[][4], int ieliel[][4])
{
     nelem = 2;

     iniel[1][1] = 3;
     iniel[1][2] = 2;
     iniel[1][3] = 1;
     iniel[2][1] = 3;
     iniel[2][2] = 4;
     iniel[2][3] = 2;

     ieliel[1][1] = 0;
     ieliel[1][2] = 0;
     ieliel[1][3] = 2;
     ieliel[2][1] = 0;
     ieliel[2][2] = 1;
     ieliel[2][3] = 0;

     return;
}

void circum(int ielem, int iniel[][4], double x[][3], double& rcs, double& xc1, double& xc2)
{
     int ip1, ip2, ip3;
     double x1, y1, x2, y2, x3, y3;
     double a11, a12, a21, a22, b1, b2;
     double deta, detb1, detb2;
     double dx, dy;

     ip1 = iniel[ielem][1];
     ip2 = iniel[ielem][2];
     ip3 = iniel[ielem][3];

     x1 = x[ip1][1];
     y1 = x[ip1][2];
     x2 = x[ip2][1];
     y2 = x[ip2][2];
     x3 = x[ip3][1];
     y3 = x[ip3][2];

     a11 = 2*(x2-x1);
     a12 = 2*(y2-y1);
     a21 = 2*(x3-x1);
     a22 = 2*(y3-y1);
     b1 = (x2*x2)+(y2*y2)-(x1*x1)-(y1*y1);
     b2 = (x3*x3)+(y3*y3)-(x1*x1)-(y1*y1);

     deta = (a11*a22)-(a12*a21);
     detb1 = (b1*a22)-(a12*b2);
     detb2 = (a11*b2)-(b1*a21);

     xc1 = detb1/deta;
     xc2 = detb2/deta;

     dx = xc1-x1;
     dy = xc2-y1;

     rcs = (dx*dx)+(dy*dy);
}

bool search(int& ielem, int nelem, int isrch, int ieliel[][4], int iniel[][4], int msrch[], double smin, double x01, double x02, double& s1, double& s2, double& s3, double x[][3])
{
     int i, je, ke, ip1, ip2, ip3;
     double x1, y1, x2, y2, x3, y3;
     double area, area1, area2, area3, areamin;

     je = ielem;
     for(i = 1; i <= nelem*nelem; i++)
     {
          msrch[je] = isrch;

          ip1 = iniel[je][1];
          ip2 = iniel[je][2];
          ip3 = iniel[je][3];

          x1 = x[ip1][1];
          y1 = x[ip1][2];
          x2 = x[ip2][1];
          y2 = x[ip2][2];
          x3 = x[ip3][1];
          y3 = x[ip3][2];

          area = ((x2-x1)*(y3-y1))-((y2-y1)*(x3-x1));
          area1 = ((x2-x01)*(y3-x02))-((y2-x02)*(x3-x01));
          area2 = ((x3-x01)*(y1-x02))-((y3-x02)*(x1-x01));
          area3 = ((x1-x01)*(y2-x02))-((y1-x02)*(x2-x01));

          areamin = area1;
          if(area2 < areamin) areamin = area2;
          if(area3 < areamin) areamin = area3;

          if(areamin >= -smin*area)
          {
               s1 = area1/area;
               s2 = area2/area;
               s3 = area3/area;
               ielem = je;

               return 0;
          }

          if(area1 < -smin*area)
          {
               ke = ieliel[je][1];
               if(msrch[ke] != isrch) je = ke;
          }
          if(area2 < -smin*area)
          {
               ke = ieliel[je][2];
               if(msrch[ke] != isrch) je = ke;
          }
          if(area3 < -smin*area)
          {
               ke = ieliel[je][3];
               if(msrch[ke] != isrch) je = ke;
          }
     }

     return 1;
}

bool find(ofstream& write, int ielem, int nelem, int& ndel, int ieliel[][4], int ielidel[], double dvtoli, double x01, double x02, double dv[], double xe[][3])
{
     int i, j, k, idel, idel1, idel2, it, jdel, ieln, jelem;
     double dx, dy, RVI, len, w, absw;

     for(i = 1; i <= nelem; i++)
     {
          dx = xe[i][1] - x01;
          dy = xe[i][2] - x02;
          len = (dx*dx) + (dy*dy);
          write<<"ielem = "<<i<<"  dv = "<<dv[i]<<"  len = "<<len<<endl;
     }
     write<<endl;

     idel = 1;
     ielidel[idel] = ielem;
     dv[ielem] = -dv[ielem];

     idel2 = 0;
     /* write<<"ielem = "<<ielem<<"  nelem = "<<nelem<<endl;
     write<<"ndel = "<<ndel<<endl;
     write<<"ieliel = "<<ieliel[ielem][1]<<"  "<<ieliel[ielem][2]<<"  "<<ieliel[ielem][3]<<endl;
     write<<"idel = "<<idel<<"  ielidel = "<<ielidel[idel]<<"  dv = "<<dv[ielem]<<endl;
     write<<"idel2 = "<<idel2<<endl; */

     for(it= 1; it <= nelem; it++)
     {
          idel1 = idel2 + 1;
          idel2 = idel;
          //write<<"  it = "<<it<<"  idel1 = "<<idel1<<"  idel2 = "<<idel2<<endl;
          for(jdel = idel1; jdel <= idel2; jdel++)
          {
               jelem = ielidel[idel];
               //write<<"    jdel = "<<jdel<<"  jelem = "<<jelem<<endl;
               for(ieln = 1; ieln <= 3; ieln++)
               {
                    ielem = ieliel[jelem][ieln];
                    //write<<"      ieln = "<<ieln<<"  ielem = "<<ielem<<endl;
                    if(ielem > 0)
                    {
                         RVI = dv[ielem];
                         //write<<"        RVI = "<<RVI<<endl;

                    if(RVI > 0)
                    {
                         dx = xe[ielem][1] - x01;
                         dy = xe[ielem][2] - x02;
                         len = (dx*dx) + (dy*dy);
                         w = len - RVI;

                         if(w < 0) absw = -w;
                         else absw = w;

                         //write<<"        dx = "<<dx<<"  dy = "<<dy<<"  len = "<<len<<endl;
                         //write<<"        w = "<<w<<"  dvtoli*RVI = "<<(dvtoli*RVI)<<endl;
                         //write<<"        absw = "<<absw<<endl;
                         if(absw > (dvtoli*RVI))
                         {
                              //write<<"          w = "<<w<<endl;
                              if(w < 0)
                              {
                                   //write<<"            idel = "<<idel<<"  ielidel = "<<ielidel[idel]<<"  dv = "<<dv[ielem]<<endl;
                                   idel++;
                                   ielidel[idel] = ielem;
                                   dv[ielem] = -RVI;
                                   //write<<"            idel = "<<idel<<"  ielidel = "<<ielidel[idel]<<"  dv = "<<dv[ielem]<<endl;
                              }
                         }
                         else return 0;
                    }
                    }
               }
          }
     }
     ndel = idel;
     return 1;
}

void insert(ofstream& write, int nnode, int ndel, int ielidel[], int ieliel[][4], double rv[], int iniel[][4], int& nadd, int inielnew[][4], int ielielnew[][4])
{
     int i, j, k, inew, ielem, idel, jeln, ieln, pt1, pt2;

     /* write<<"checking..."<<endl;
     for(i = 1; i <= 8; i++)
     {
          write<<"i = "<<i<<"  dv = "<<rv[i]<<endl;
     }
     write<<endl; */

     inew = 0;
     //write<<"inew = "<<inew<<endl;
     for(idel = 1; idel <= ndel; idel++)
     {
          ielem = ielidel[idel];
          //write<<"  idel = "<<idel<<"  ielidel = "<<ielem<<endl;
          for(jeln = 1; jeln <= 3; jeln++)
          {
               ieln = ieliel[ielem][jeln];
               //write<<"    ieln = "<<ieln<<"  rv = "<<rv[ieln]<<endl;
               if((ieln <= 0) || (rv[ieln] > 0))
               {
                    inew++;
                    //write<<"      inew = "<<inew<<endl;

                    if(jeln == 1)
                    {
                         pt1 = 2;
                         pt2 = 3;
                    }
                    else if(jeln == 2)
                    {
                         pt1 = 3;
                         pt2 = 1;
                    }
                    else
                    {
                         pt1 = 1;
                         pt2 = 2;
                    }

                    inielnew[inew][1] = iniel[ielem][pt1];
                    inielnew[inew][2] = iniel[ielem][pt2];
                    inielnew[inew][3] = nnode;

                    ielielnew[inew][1] = -1;
                    ielielnew[inew][2] = -1;
                    ielielnew[inew][3] = ieliel[ielem][jeln];

                    //write<<"      inielnew1 = "<<inielnew[inew][1]<<endl;
                    //write<<"      inielnew2 = "<<inielnew[inew][2]<<endl;
                    //write<<"      inielnew3 = "<<inielnew[inew][3]<<endl;
               }
          }
     }
     write<<endl;
     nadd = inew;

     for(i = 1; i <= nadd; i++)
     {
          for(j = 1; j <= nadd; j++)
          {
               if(i != j)
               {
                    //write<<"i = "<<i<<"  j = "<<j<<endl;
                    for(k = 1; k <= 2; k++)
                    {
                         if(inielnew[i][k] == inielnew[j][3-k])
                         {
                              ielielnew[i][3-k] = j;
                         }
                    }
               }
          }
     }

     /* for(i = 1; i <= nadd; i++)
     {
          write<<"inewaft = "<<i<<endl;
          write<<"  ielielnew1 = "<<ielielnew[i][1]<<endl;
          write<<"  ielielnew2 = "<<ielielnew[i][2]<<endl;
          write<<"  ielielnew3 = "<<ielielnew[i][3]<<endl;
     } */

     return;
}

void rebuild(ofstream& write, int nadd, int ndel, int& nelem, int iniel[][4], int ieliel[][4], int inielnew[][4], int ielielnew[][4], int ielidel[], double dv[], double xe[][3])
{
     int i, j, k, iii, nelemtemp, val, tempval;
     int tiniel[100][4], tieliel[100][4], ifnmap[100];
     double tdv[100], txe[100][3];

     write<<"nelem = "<<nelem<<endl<<endl;
     for(i = 1; i <= nelem; i++)
     {
          write<<"ielem = "<<i<<endl;
          write<<"  iniel1 = "<<iniel[i][1]<<endl;
          write<<"  iniel2 = "<<iniel[i][2]<<endl;
          write<<"  iniel3 = "<<iniel[i][3]<<endl;
          write<<"  ieliel1 = "<<ieliel[i][1]<<endl;
          write<<"  ieliel2 = "<<ieliel[i][2]<<endl;
          write<<"  ieliel3 = "<<ieliel[i][3]<<endl;
     }
     write<<endl;

     write<<"ndel = "<<ndel<<endl<<endl;
     for(i = 1; i <= ndel; i++)
     {
          write<<"i = "<<i<<"  ielidel = "<<ielidel[i]<<endl;
     }
     write<<endl;

     write<<"nadd = "<<nadd<<endl<<endl;
     for(i = 1; i <= nadd; i++)
     {
          write<<"iadd = "<<i<<endl;
          write<<"  iniel1 = "<<inielnew[i][1]<<endl;
          write<<"  iniel2 = "<<inielnew[i][2]<<endl;
          write<<"  iniel3 = "<<inielnew[i][3]<<endl;
          write<<"  ieliel1 = "<<ielielnew[i][1]<<endl;
          write<<"  ieliel2 = "<<ielielnew[i][2]<<endl;
          write<<"  ieliel3 = "<<ielielnew[i][3]<<endl;
     }
     write<<endl;

     write<<"adding..."<<endl;
     for(i = 1; i <= nadd; i++)
     {
          nelem++;
          iniel[nelem][1] = inielnew[i][1];
          iniel[nelem][2] = inielnew[i][2];
          iniel[nelem][3] = inielnew[i][3];

          ieliel[nelem][1] = ielielnew[i][1];
          ieliel[nelem][2] = ielielnew[i][2];
          ieliel[nelem][3] = ielielnew[i][3];
          dv[nelem] = 1.0;
     }
          write<<"nelem = "<<nelem<<endl<<endl;
          for(i = 1; i <= nelem; i++)
          {
               write<<"ielem = "<<i<<endl;
               write<<"  iniel1 = "<<iniel[i][1]<<endl;
               write<<"  iniel2 = "<<iniel[i][2]<<endl;
               write<<"  iniel3 = "<<iniel[i][3]<<endl;
               write<<"  ieliel1 = "<<ieliel[i][1]<<endl;
               write<<"  ieliel2 = "<<ieliel[i][2]<<endl;
               write<<"  ieliel3 = "<<ieliel[i][3]<<endl;
          }
          write<<endl;

     write<<"reconnecting..."<<endl;
     k = nelem-nadd+1;
     for(i = 1; i <= nadd; i++)
     {
          for(j = 1; j <= 2; j++)
          {
               ieliel[i+k-1][j] = ieliel[i+k-1][j] + k-1;
          }
     }
          write<<"nelem = "<<nelem<<endl<<endl;
          for(i = 1; i <= nelem; i++)
          {
               write<<"ielem = "<<i<<endl;
               write<<"  iniel1 = "<<iniel[i][1]<<endl;
               write<<"  iniel2 = "<<iniel[i][2]<<endl;
               write<<"  iniel3 = "<<iniel[i][3]<<endl;
               write<<"  ieliel1 = "<<ieliel[i][1]<<endl;
               write<<"  ieliel2 = "<<ieliel[i][2]<<endl;
               write<<"  ieliel3 = "<<ieliel[i][3]<<endl;
          }
          write<<endl;

          write<<"nelem = "<<nelem<<endl<<endl;
          for(i = 1; i <= nelem; i++)
          {
               write<<"ielem = "<<i<<endl;
               write<<"  iniel1 = "<<iniel[i][1]<<endl;
               write<<"  iniel2 = "<<iniel[i][2]<<endl;
               write<<"  iniel3 = "<<iniel[i][3]<<endl;
               write<<"  ieliel1 = "<<ieliel[i][1]<<endl;
               write<<"  ieliel2 = "<<ieliel[i][2]<<endl;
               write<<"  ieliel3 = "<<ieliel[i][3]<<endl;
          }
          write<<endl;
     write<<"deleting..."<<endl;
     iii = 1;
     for(i = 1; i <= nelem; i++)
     {
          ifnmap[i] = iii;
          write<<"i = "<<i<<"  dv = "<<dv[i]<<endl;
          if(dv[i] > 0.0)
          {
               tiniel[iii][1] = iniel[i][1];
               tiniel[iii][2] = iniel[i][2];
               tiniel[iii][3] = iniel[i][3];

               tdv[iii] = dv[i];
               txe[iii][1] = xe[i][1];
               txe[iii][2] = xe[i][2];

               write<<"   val = "<<nelem-nadd+1<<endl;
               if(i >= (nelem-nadd+1))
               {
                    tieliel[iii][1] = ieliel[i][1]-(i-iii);
                    tieliel[iii][2] = ieliel[i][2]-(i-iii);
                    write<<"      val2 = "<<nelem-ndel-nadd+1<<endl;
                    if(ieliel[i][3] >= (nelem-ndel-nadd+1))
                    {
                         write<<"         aaa"<<endl;
                         val = ieliel[i][3];
                         tieliel[iii][3] = ifnmap[val];
                    }
                    else
                    {
                         write<<"         bbb"<<endl;
                         tieliel[iii][3] = ieliel[i][3];
                    }

               }
               else
               {
                    write<<"      ccc"<<endl;
                    tieliel[iii][1] = ieliel[i][1];
                    tieliel[iii][2] = ieliel[i][2];
                    tieliel[iii][3] = ieliel[i][3];
               }
               iii++;
          }
     }
     write<<endl;
     for(i = 1; i <= nelem; i++) write<<"ifnmap["<<i<<"] = "<<ifnmap[i]<<endl;
     write<<endl;
     for(i = 1; i <= nelem-1; i++)
     {
          if(ifnmap[i] == ifnmap[i+1])
          {
               tempval = i;
          }
     }
     write<<"   tempval = "<<tempval<<endl;
     nelem = nelem-ndel;
     for(i = 1; i <= nelem; i++)
     {
               iniel[i][1] = tiniel[i][1];
               iniel[i][2] = tiniel[i][2];
               iniel[i][3] = tiniel[i][3];
               ieliel[i][1] = tieliel[i][1];
               ieliel[i][2] = tieliel[i][2];
               ieliel[i][3] = tieliel[i][3];
               dv[i] = tdv[i];
               xe[i][1] = txe[i][1];
               xe[i][2] = txe[i][2];
     }
          write<<"nelem = "<<nelem<<endl<<endl;
          for(i = 1; i <= nelem; i++)
          {
               write<<"ielem = "<<i<<endl;
               write<<"  iniel1 = "<<iniel[i][1]<<endl;
               write<<"  iniel2 = "<<iniel[i][2]<<endl;
               write<<"  iniel3 = "<<iniel[i][3]<<endl;
               write<<"  ieliel1 = "<<ieliel[i][1]<<endl;
               write<<"  ieliel2 = "<<ieliel[i][2]<<endl;
               write<<"  ieliel3 = "<<ieliel[i][3]<<endl;
          }
          write<<endl;
     write<<"re-reconntecting..."<<endl;

     for(i = 1; i <= nelem-nadd; i++)
     {
          for(j = 1; j <= 3; j++)
          {
               if(ieliel[i][j] >= tempval)
               {
                    write<<"change this value"<<endl;
               }
          }
     }

     write<<endl;
}

