       subroutine wrelp(a,b,alpha)
C.....subroutine  wrelp   01/06/75      01/01/80
      common/gfgel/gf(28)
      common/gfbftr/xtr,ytr
      gf(10)=100.
      i=abs(gf(9))
      gf(i)=-2.
      gf(i+1)=xtr
      gf(i+2)=ytr
      gf(i+3)=a
      gf(i+4)=b
      gf(i+5)=alpha
      gf(i+6)=0.
      gf(i+7)=360.
      if(gf(9).gt.0.) call strmod(0)
      return
      end
