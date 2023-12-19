%% Modelado de la planta
close all;
load 1Np10.txt
tiempo = X1Np10(:,1);
entrada = X1Np10(:,2);
salida = X1Np10(:,3);
%% Plots
close all
% Tiempo para 2 V
% t_ini = 2.283;
% t_fin = 5.172;
% Tiempo para 4 v
% t_ini = 2.2310;
% t_fin = 4.056;
% Tiempo deseados
t_ini = 4.741;
t_fin = 9.351;
x_ini = find(tiempo == t_ini);
x_fin = find(tiempo == t_fin);
tiempo_interes = tiempo(x_ini:x_fin) - tiempo(x_ini);
salida_interes = salida(x_ini:x_fin);
plot(tiempo_interes,salida_interes)
hold on
plot(ScopeData(:,1), ScopeData(:,2));
title ("Respuesta del sistema bajando con 10 personas");
legend(["Sistema real" "Sistema simulado"])
ylabel("Velocidad (m/s)");
xlabel("Tiempo (s)");

%% Plot del sistema
close all
subplot(2,1,1)
plot(ScopeData(:,1), ScopeData(:,2));
hold on
plot([0 4], [1 1]*1,'k:')
plot([0 4], [1 1]*1*0.95,'r:')
plot([0 4], [1 1]*1*1.05,'r:')
title ("Respuesta del sistema subiendo");
ylabel("Velocidad (m/s)");
xlabel("Tiempo (s)");

subplot(2,1,2)
plot(ScopeData1(:,1), ScopeData1(:,2));
hold on
plot([0 4], [1 1]*-1,'k:')
plot([0 4], [1 1]*-1*0.95,'r:')
plot([0 4], [1 1]*-1*1.05,'r:')
title ("Respuesta del sistema subiendo");
ylabel("Velocidad (m/s)");
xlabel("Tiempo (s)");
%% Equivalente discreto
close all
T = 20e-3;
Gs = tf(0.1405, [0.251 1]);
BGz = c2d(Gs,T,'zoh');
F = tf([1 1],[1 -1],T);
CD = series(BGz,F)
rlocus(CD)
hold on
for ts = [0.9]
    ks = ceil(ts/T)
    UnitCircle(exp(-pi/ks))
end
M = feedback(CD,1);
[num, den] = zpkdata(M,'v');
%%
[c,t] = step(BGz);
c_inf = c(end);
figure
stairs(t,c)
hold on
plot([t(1) t(end)], [1 1]*c_inf,'k:')
plot([t(1) t(end)], [1 1]*c_inf*0.95,'r:')
plot([t(1) t(end)], [1 1]*c_inf*1.05,'r:')
