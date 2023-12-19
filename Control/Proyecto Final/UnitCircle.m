function UnitCircle(r)
% Dibuja la circunferencia de radio unidad
% Pero si se indica r dibuja una circunferencia
% de radio r 

if nargin==0,r=1;end

i=1;
for a=0:.05:2*pi;
    xc(i)=r*cos(a);
    yc(i)=r*sin(a);
    i=i+1;end
hold on,
plot(xc,yc,'r:')