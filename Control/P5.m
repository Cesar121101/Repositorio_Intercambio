clear all
close all
Gv = tf(0.3,[0.03 1])
figure('name', ['LDR con CA(s) = K*Gv(s)'])
rlocus(Gv)
hold on
for ts = [0.075 0.6]
    plot([-1 -1]*pi/ts, [-1 1]*5, 'r:')
end
axis([-120 10 -5 5])
figure
step(Gv)

%%
hold off
for K = [0.85];
    M = feedback(K*Gv,1)
    figure('name', ['Respuesta M para K = ' num2str(K)])
    [c,t] = step(M);
    c_inf = c(end);
    plot(t,c)
    hold on
    plot([t(1) t(end)], [1 1]*c_inf,'k:')
    plot([t(1) t(end)], [1 1]*c_inf*0.95,'r:')
    plot([t(1) t(end)], [1 1]*c_inf*1.05,'r:')
end

%%
T = 5e-3;
BGv = c2d(Gv,T,'zoh')
figure('name', 'LDR con CA(z) = K*BGv(z)')
rlocus(BGv)
hold on
for ts = [0.075 0.6]
    ks = ceil(ts/T)
    UnitCircle(exp(-pi/ks))
end
axis([-1 1 -1 1])
%%
for K = [0.77 36 39.5];
    M = feedback(K*BGv,1)
    figure('name', ['Respuesta M para K=' num2str(K) ' y T=' num2str(T)])
    [c,t] = step(M);
    c_inf = c(end);
    stairs(t,c)
    hold on
    plot([t(1) t(end)], [1 1]*c_inf,'k:')
    plot([t(1) t(end)], [1 1]*c_inf*0.95,'r:')
    plot([t(1) t(end)], [1 1]*c_inf*1.05,'r:')
end
%%
Gp = tf(4,[0.03 1 0])
figure('name','LDR con CA(s) = K* Gp(s)')
rlocus(Gp)
hold on
for tp = [0.09 0.195]
    plot([-120 10], [1 1]*pi/tp,'r:')
end
axis([-120 10 -50 50])
figure
step(Gp)
%%
for K = [1.1];
    M = feedback(K*Gp,1)
    figure('name', ['Respuesta M para K = ' num2str(K)])
    [c,t] = step(M);
    c_inf = c(end);
    plot(t,c)
    hold on
    plot([t(1) t(end)], [1 1]*c_inf,'k:')
    plot([t(1) t(end)], [1 1]*c_inf*0.95,'r:')
    plot([t(1) t(end)], [1 1]*c_inf*1.05,'r:')
end
%%
T = 5e-3
BGp = c2d(Gp,T,'zoh')
figure('name','LDR con CA(z) = K*BGp(z)')
rlocus(BGp)
hold on
for tp = [0.09 0.195]
    kp = round(tp/T);
    plot([0 1], [0 tan(pi/kp)],'r:')
end
axis([-1 1 -1 1])
%%
for K = [4.03 11.2];
    M = feedback(K*BGv,1)
    figure('name', ['Respuesta M para K=' num2str(K) ' y T=' num2str(T)])
    [c,t] = step(M);
    c_inf = c(end);
    stairs(t,c)
    hold on
    plot([t(1) t(end)], [1 1]*c_inf,'k:')
    plot([t(1) t(end)], [1 1]*c_inf*0.95,'r:')
    plot([t(1) t(end)], [1 1]*c_inf*1.05,'r:')
end
%%
clear all
close all
Gv = tf(500,[0.5 1])
figure('name', ['LDR con CA(s) = K*Gv(s)'])
rlocus(Gv)
% hold on
% for ts = [0.075 0.6]
%     plot([-1 -1]*pi/ts, [-1 1]*5, 'r:')
% end
% axis([-120 10 -5 5])
% figure
% step(Gv)