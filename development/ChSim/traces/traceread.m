files = dir;
nFiles = size(files);
for x = 1:nFiles(1)
    if (findstr(files(x).name, '.trace')>1)
        figure;
        hold on;
        filename = [files(x).name]
        ms = dlmread(filename);
        %2D
        plot(ms);
        hold off;
        ylabel('Channel state [dBm]');
        xlabel('Samples');
        %3D
        figure;
        colormap hsv;
        shading faceted;
        surf(ms, 'EdgeColor', 'none');
        xlabel('Sub bands');
        ylabel('Samples');
        zlabel('Channel state [dB]');
    end
end
