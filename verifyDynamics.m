%% Ben Conrad -- Verify the Sliding Block Dynamics -- 20160303
clc; clear all; close all; format compact;

%% Load
res = importdata('verifyDynamics.csv'); %has columns [time][x1,y1,z1],...[xn,yn,zn]
N = (size(res,2)-1)/3; %remove time, then xyz
t = res(:,1);
ixs = (1:3:(3*N)) + 1;
iys = ixs+1;
izs = ixs+2;

%% Properties
blockM = 2; %kg
springK = 10; %N/m
springL = 10; %m
dampC = .1; %N/m/s
A = 4.4644; % initial velocity != force impulse, so eyeball it or pull from envp1.a (below)
wn = sqrt(springK/blockM); %[rad/s]
zeta = dampC / ( 2*sqrt(springK*blockM) );
wd = wn*sqrt(1-zeta^2);

%% Measure
[ipks,ivls] = findPeaks(res(:,2));
td = mean( diff(t(ipks)) ); %time (damped) between peaks
fprintf('Comparing damped natural frequency: wd = %3.5f vs 2pi/td = %3.5f\n', wd, 2*pi/td);
envp1 = fit(t(ipks),res(ipks,2)-springL,'exp1'); %fit the peak envelope
envn1 = fit(t(ivls),res(ivls,2)-springL,'exp1'); %fit the valleys

%% Plot
close all;
figure; hold on; 
plot( t, res(:,2), 'k-');
plot( t(ipks), res(ipks,2), 'bx');
plot( t(ivls), res(ivls,2), 'bx');
plot( t, springL + A*exp( -zeta*wn*t ), 'c-' );
plot( t, springL - A*exp( -zeta*wn*t ), 'c-' );
plot( t, springL + envp1.a*exp( envp1.b * t ), 'r--' );
plot( t, springL + envn1.a*exp( envn1.b * t ), 'r--' );
xlabel('Time [s]'); ylabel('Block Position [m]');

