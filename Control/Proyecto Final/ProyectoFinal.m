%% Modelado de la planta
close all;
load 0.5Np10.txt
tiempo = X0_5Np10(:,1);
entrada = X0_5Np10(:,2);
salida = X0_5Np10(:,3);
%% Plots
close all
% Tiempo para 2 V
% t_ini = 2.283;
% t_fin = 5.172;
% Tiempo para 4 v
% t_ini = 2.2310;
% t_fin = 4.056;
% Tiempo deseados
t_ini = 6.389;
t_fin = 10.147;
x_ini = find(tiempo == t_ini);
x_fin = find(tiempo == t_fin);
tiempo_interes = tiempo(x_ini:x_fin) - tiempo(x_ini);
salida_interes = salida(x_ini:x_fin);
plot(tiempo_interes,salida_interes);
hold on
plot(ScopeData(:,1), ScopeData(:,2));
legend(["Sistema real" "Sistema simulado"])
title ("Respuesta del sistema con controlador Truxal");
ylabel("Velocidad (m/s)");
xlabel("Tiempo (s)");
% axis([0 2 0 0.7])
% axis([0 3 0 0.35])

%% Plot del sistema
close all
plot(ScopeData(:,1), ScopeData(:,2));
title ("Respuesta del sistema con controlador Truxal");
ylabel("Velocidad (m/s)");
xlabel("Tiempo (s)");
%% Equivalente discreto
close all
T = 20e-3;
Gs = tf(0.1405, [0.251 1]);
BGz = c2d(Gs,T,'zoh')
F = tf([6.05,-5.5866],[1 -1],T)
CD = series(BGz,F)
rlocus(CD)
M = feedback(CD,1)
%%
[c,t] = step(BGz);
c_inf = c(end);
figure
stairs(t,c)
hold on
plot([t(1) t(end)], [1 1]*c_inf,'k:')
plot([t(1) t(end)], [1 1]*c_inf*0.95,'r:')
plot([t(1) t(end)], [1 1]*c_inf*1.05,'r:')
