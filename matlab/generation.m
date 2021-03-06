function generation( tn, pn, ut, tsnum )
%GENERATION Summary of this function goes here
%   Detailed explanation goes here
prefix='tasksets/';
x='_';
filename=prefix;
filename=strcat(filename, int2str(tn));
filename=strcat(filename, x);
filename=strcat(filename, int2str(pn));
filename=strcat(filename, x);
filename=strcat(filename, int2str(ut));
for i=1:tsnum
    filename1=strcat(filename, x);
    filename1=strcat(filename1, int2str(i));
    fout=fopen(filename1, 'w');
    [exe, pri, res, hp]=generateTaskset(tn, pn, 5, 5, 30, ut/100);
    while(res==0)
        [exe, pri, res, hp]=generateTaskset(tn, pn, 5, 5, 30, ut/100);
    end
    fprintf(fout, '%s %s %s %s\n', int2str(tn), int2str(length(exe)), int2str(pn), int2str(hp));
    for i=1:length(pri)
        fprintf(fout, '%s ', int2str(pri(i)));
    end
    fprintf(fout, '\n');
    for i=1:length(exe)
        fprintf(fout, '%s ', int2str(exe(i)));
    end
    fprintf(fout, '\n');
    fclose(fout);
end

end

