%% Modelado de la planta
close all;
load datos4V.txt
tiempo = datos4V(:,1);
entrada = datos4V(:,2);
salida = datos4V(:,3);
% Tiempo para 2 V
% t_ini = 2.283;
% t_fin = 5.172;
% Tiempo para 4 v
t_ini = 2.2310;
t_fin = 4.056;
% Tiempo para perturbacion -1V
% t_ini = 2.734;
% t_fin = 5.345;
x_ini = find(tiempo == t_ini);
x_fin = find(tiempo == t_fin);
tiempo_interes = tiempo(x_ini:x_fin) - tiempo(x_ini);
salida_interes = salida(x_ini:x_fin);
plot(tiempo_interes,salida_interes);
hold on
plot(ScopeData(:,1), ScopeData(:,2));
title ("Respuesta de los sistemas a escalon de 4 V");
legend(["Sistema real" "Funcion de transferencia obtenida"])
ylabel("Velocidad (m/s)");
xlabel("Tiempo (s)");
axis([0 2 0 0.7])
% axis([0 3 0 0.35])

%% Plot del sistema
plot(ScopeData(:,1), ScopeData(:,2));
title ("Respuesta del sistema con controlador Truxal");
ylabel("Velocidad (m/s)");
xlabel("Tiempo (s)");
hold on
plot(ScopeData1(:,1),ScopeData1(:,2)* 1,'k:');
plot(ScopeData1(:,1),ScopeData1(:,2)*0.95,'r:');
plot(ScopeData1(:,1),ScopeData1(:,2)*1.05,'r:');

%% Equivalente discreto
close all
T = 20e-3;
Gs = tf(0.1405, [0.251 1]);
BGz = c2d(Gs,T,'zoh');
F = tf([0.08771 -0.08099],[0.01076 -0.01074],T)
CD = series(BGz,F);
M = feedback(CD,1)
figure
rlocus(M)
%%
[c,t] = step(BGz);
c_inf = c(end);
figure
stairs(t,c)
hold on
plot([t(1) t(end)], [1 1]*c_inf,'k:')
plot([t(1) t(end)], [1 1]*c_inf*0.95,'r:')
plot([t(1) t(end)], [1 1]*c_inf*1.05,'r:')
