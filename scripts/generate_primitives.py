from pathlib import Path
import math
out=Path(__file__).resolve().parents[1]/'gameApp/models'
def surface(name,rings):
    lines=['# Fersa generated primitive'];n=32
    for j,(r,y,ny,nr) in enumerate(rings):
        for i in range(n+1):
            a=2*math.pi*i/n;c,s=math.cos(a),math.sin(a)
            lines += [f'v {r*c:.7f} {y:.7f} {r*s:.7f}',f'vt {i/n:.7f} {j/(len(rings)-1):.7f}',f'vn {nr*c:.7f} {ny:.7f} {nr*s:.7f}']
    for j in range(len(rings)-1):
        if rings[j][:2]==rings[j+1][:2]:continue
        for i in range(n):
            a=j*(n+1)+i+1;b=a+n+1
            faces=[]
            if rings[j][0]>1e-6:faces.append((a,a+1,b))
            if rings[j+1][0]>1e-6:faces.append((a+1,b+1,b))
            for face in faces:lines.append('f '+' '.join(f'{v}/{v}/{v}' for v in reversed(face)))
    (out/f'{name}.obj').write_text('\n'.join(lines)+'\n')
for name,h in [('sphere',0),('capsule',.5)]:
    rings=[]
    for j in range(17):
        a=-math.pi/2+math.pi*j/16
        if h and j==8:rings.extend([(.5,-h,0,1),(.5,h,0,1)])
        else:rings.append((math.cos(a)*.5,math.sin(a)*.5+(-h if j<8 else h),math.sin(a),math.cos(a)))
    surface(name,rings)
surface('cylinder',[(0,-.5,-1,0),(.5,-.5,-1,0),(.5,-.5,0,1),(.5,.5,0,1),(.5,.5,1,0),(0,.5,1,0)])
surface('cone',[(0,-.5,-1,0),(.5,-.5,-1,0),(.5,-.5,1/math.sqrt(5),2/math.sqrt(5)),(0,.5,1/math.sqrt(5),2/math.sqrt(5))])
for name,vs in [('square',[(-.5,-.5,0),(.5,-.5,0),(.5,.5,0),(-.5,.5,0)]),('plane',[(-1,0,1),(1,0,1),(1,0,-1),(-1,0,-1)])]:
    (out/f'{name}.obj').write_text('\n'.join(['# Fersa primitive']+['v %s %s %s'%v for v in vs]+['vt 0 0','vt 1 0','vt 1 1','vt 0 1','f 1/1 2/2 3/3 4/4'])+'\n')

