%% Modelado de la planta
close all;
load datos4V.txt
tiempo = datos2V(:,1);
entrada = datos2V(:,2);
salida = datos2V(:,3);
t_ini = 2.283;
t_fin = 5.172;
% t_ini = 2.2310;
% t_fin = 4.056;
x_ini = find(tiempo == t_ini);
x_fin = find(tiempo == t_fin);
tiempo_interes = tiempo(x_ini:x_fin) - tiempo(x_ini);
salida_interes = salida(x_ini:x_fin);
plot(tiempo_interes,salida_interes);
hold on
plot(ScopeData(:,1), ScopeData(:,2));

%% Equivalente discreto
T = 20e-3;
Gs = tf([0.1405], [0.251 1])
Gz = c2d(Gs,T,'zoh')
[c,t] = step(Gz);
c_inf = c(end);
figure
stairs(t,c)
hold on
plot([t(1) t(end)], [1 1]*c_inf,'k:')
plot([t(1) t(end)], [1 1]*c_inf*0.95,'r:')
plot([t(1) t(end)], [1 1]*c_inf*1.05,'r:')
