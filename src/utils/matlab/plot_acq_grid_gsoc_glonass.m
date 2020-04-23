%  Reads GNSS-SDR Acquisition dump binary file using the provided
%  function and plots acquisition grid of acquisition statistic of PRN sat
%
%  This function analyzes a experiment performed by Luis Esteve in the framework
%  of the Google Summer of Code (GSoC) 2012, with the collaboration of Javier Arribas
%  and Carles Fernandez, related to the extension of GNSS-SDR to Galileo.
%
% -------------------------------------------------------------------------
%
% Copyright (C) 2010-2019  (see AUTHORS file for a list of contributors)
%
% GNSS-SDR is a software defined Global Navigation
%           Satellite Systems receiver
%
% This file is part of GNSS-SDR.
% 
% SPDX-License-Identifier: GPL-3.0-or-later
%
% -------------------------------------------------------------------------
%

function plot_acq_grid_gsoc_glonass(sat)

file=['/archive/acquisition_R_1G_sat_' num2str(sat) '_doppler_0.dat'];

% sampling_freq_Hz=62316000
sampling_freq_Hz=6.625e6
Doppler_max_Hz = 10000
Doppler_min_Hz = -10000
Doppler_step_Hz = 250


% read files

x=read_complex_binary (file);

l_y=length(x);

Doppler_axes=Doppler_min_Hz:Doppler_step_Hz:Doppler_max_Hz;

l_x=length(Doppler_axes);

acq_grid = zeros(l_x,l_y);

index=0;

for k=Doppler_min_Hz:Doppler_step_Hz:Doppler_max_Hz
    index=index+1;
    filename=['acquisition_R_1G_sat_' num2str(sat) '_doppler_' num2str(k) '.dat'];
    acq_grid(index,:)=abs(read_complex_binary (filename));
end

acq_grid = acq_grid.^2;

maximum_correlation_peak = max(max(acq_grid))

[fila,col]=find(acq_grid==max(max(acq_grid)));

delay_error_sps = col -1

Doppler_error_Hz = Doppler_axes(fila)

noise_grid=acq_grid;
delay_span=floor(3*sampling_freq_Hz/(0.511e6));
Doppler_span=floor(500/Doppler_step_Hz);
noise_grid(fila-Doppler_span:fila+Doppler_span,col-delay_span:col+delay_span)=0;

n=numel(noise_grid)-(2*delay_span+1)*(2*Doppler_span+1);

noise_floor= sum(sum(noise_grid))/n

Gain_dbs = 10*log10(maximum_correlation_peak/noise_floor)


%% Plot 3D FULL RESOLUTION


[X,Y] = meshgrid(Doppler_axes,1:1:l_y);
figure;
surf(X,Y,acq_grid');

xlabel('Doppler(Hz)');ylabel('Code Delay(samples)');title(['GLRT statistic of Glonass Parallel Code Phase Search Acquisition. Local replica: L1 cboc PRN ' num2str(sat)]);


end
