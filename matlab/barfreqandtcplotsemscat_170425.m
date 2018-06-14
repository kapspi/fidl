%/data/emotion2/data21/raichle/np600_analysis_new/modellongclosedandopen/constrainedwstates/np645reg/freqplot_060719.m
gray=[.55 .55 .55];
graylight=[.69 .69 .69];
graydark=[.41 .41 .41];

subject_title = 'det'
filename = 'fidl_ttest_region_results_meansem.txt'
subject = 'L-R'

%subject_title = 'sto'
%filename = 'fidl_bolds.txt'

%ncond = 6;
%ncond = 8;
%ncond = 4;
%ncond = 18;
ncond = 3;
%ncond = 9;

%voxcnt=1;
voxcnt=0;

%separator = 'Hz'
separator = '_';

%freqs_are = 1 %'interspersed'
freqs_are = 2; %'stacked'

%plot=0 bar=1 scatter=2
plotbar=1;

fid=fopen(filename);
line = fgetl(fid);
regnames = '';
nvox = '';
while(any(line))
    [T1,line] = strtok(line);

    %[T2,line] = strtok(line);
    %START151029
    if voxcnt==1
        [T2,line] = strtok(line);
    end

    regnames = strvcat(regnames,sprintf('%s',T1));

    %nvox = strvcat(nvox,sprintf('%s',T2));
    %START151029
    if voxcnt==1
        nvox = strvcat(nvox,sprintf('%s',T2));
    end

end
regnames = strrep(cellstr(regnames),'_',' ');
nvox = strrep(cellstr(nvox),'_',' ');
[nreg,junk]=size(regnames)
condstr = '';

%START140910
%groupstr = '';

freq = '';
data = '';
while 1
    line = fgetl(fid);
    if ~ischar(line), break, end
    R = strrep(line,separator,' ');
    [T1,R] = strtok(R);
    [T2,R] = strtok(R);

    %START140910
    %[T3,R] = strtok(R);

    freq = strvcat(freq,T1);
    condstr = strvcat(condstr,T2);

    %START140910
    %groupstr = strvcat(groupstr,T3);

    data = strvcat(data,R);
end
fclose(fid);
condstr = cellstr(condstr)

%START140910
%groupstr = cellstr(groupstr)

freq = str2num(freq);
data = str2num(data);
mindata = min(min(data))
maxdata = max(max(data))
[m,ncol] = size(data);
if ncol==(2*nreg)
    lcsem=1
else
    lcsem=0
end
ndataeach = m/ncond;
[m,ncol] = size(data)

%ndataeach = m/ncond
%START170419
if plotbar~=2
    ndataeach=m/ncond
end

if freqs_are == 1 %'interspersed'
    i1 = zeros(1,ndataeach);
    j = 1;
    for i=1:ndataeach
        i1(i) = j;
        j = j + ncond;
    end
    freq1 = freq(i1);
    ylabel = char('Root mean square amplitude power','        (percent change)        ');
elseif freqs_are == 2 %'stacked'

    %freq1 = freq(1:ndataeach);
    %index = zeros(ncond,ndataeach);
    %for i=1:ncond
    %    index(i,:) = [(i-1)*ndataeach+1:i*ndataeach];
    %end
    %START170419
    if plotbar~=2
        freq1=freq(1:ndataeach);
        index = zeros(ncond,ndataeach);
        for i=1:ncond
            index(i,:) = [(i-1)*ndataeach+1:i*ndataeach];
        end
    end
    
    ylabel = 'evoked response (percent change)';
end
miny = min(data);
maxy = max(data);
plotter = {'ro-','gs-'};
 
for i=1:nreg
%for i=1:1

  %if i==3|i==8|i==29|i==30
  %if i==2|i==3|i==8|i==29

    if freqs_are == 1 %'interspersed'
        fix = data(i1,i);
        open = data(i1+1,i);
        closed = data(i1+2,i);
        figure
        plot(freq1,fix,'ro-','markersize',8,'linewidth',3), hold on
        plot(freq1,open,'gs-','markersize',8,'linewidth',3), hold on
        plot(freq1,closed,'bd-','markersize',8,'linewidth',3), hold on
        set(gca,'XScale','log','Yscale','log','XLim',[2.8e-3 0.14],'YLim',[0.075 1.7]','FontSize',16,...
           'Xlabel',text('String','Frequency (Hz)','FontSize',16), ...
           'Ylabel',text('String',ylabel,'FontSize',16))
        %text(.01,1,regnames(i),'Color','k','FontSize',20)
        text(.005,1,regnames(i),'Color','k','FontSize',20)
        text(4e-3,0.2,'fixation','Color','r','FontSize',16)
        text(4e-3,0.17,'open','Color','g','FontSize',16)
        text(4e-3,0.14,'closed','Color','b','FontSize',16)
        plot(3.5e-3,0.2025,'ro','markersize',8,'linewidth',3), hold on
        plot(3.5e-3,0.1725,'gs','markersize',8,'linewidth',3), hold on
        plot(3.5e-3,0.1425,'bd','markersize',8,'linewidth',3)
        title(subject_title);
        %y = strcat(subject,blanks(1),deblank(regnames(i,:)))
        y = strcat(subject,'_',deblank(regnames(i,:)))
        print('-dtiff',char(strrep(y,' ','_')),'-r300')
    elseif freqs_are == 2 %'stacked'
        j=i;
        if lcsem==1
            j=2*i-1;
        end
        if plotbar == 0
            colors0=zeros(ncond,3);
            colors0(1,:)=[0 0 1]; %blue
            colors0(2,:)=[0 1 1]; %cyan
            colors0(3,:)=[1 0 0]; %red
            colors0(4,:)=[1 0 1]; %magenta
            colors0(5,:)=[0 1 0]; %green
            colors0(6,:)=[.29 .46 .27]; %artichoke green
            figure
            for k=1:ncond
                plot(freq1,data(index(k,:),j),'o-','markersize',12,'linewidth',3,'Color',colors0(k,:),'MarkerEdgeColor',...
                    colors0(k,:),'MarkerFaceColor',colors0(k,:)), hold on
            end
            if lcsem==1
                for k=1:ncond
                    h=errorbar(freq1,data(index(k,:),j),data(index(k,:),j+1));
                    set(h,'Color',colors0(k,:),'markersize',12,'linewidth',1,'linestyle','-')
                end
            end
            set(gca,'XLim',[freq(1)-1 freq(ndataeach)+1],'linewidth',2,'FontSize',24,'ticklength',[.02 .02],'box','off',...
                'tickdir','out')
            label1='';

            %text(0,.47,strcat(condstr(index(1,1)),label1),'Color','blue','FontSize',24)
            %text(0,.39,strcat(condstr(index(2,1)),label1),'Color','red','FontSize',24)
            %if ncond==3
            %    text(0,.31,strcat(condstr(index(3,1)),label1),'Color','green','FontSize',24)
            %end
            %text(15,.5,regnames(i),'Color','black','FontSize',24)
            %text(20,.47,nvox(i),'Color','black','FontSize',18)
            %text(7.5,.39,subject,'Color','black','FontSize',18)
            %title(regnames(i),'FontSize',24);

            y = sprintf('%s %s',subject,char(deblank(regnames(i,:))));
            title(y,'FontSize',12);
        elseif plotbar==1
            Y=data(:,j);
            if lcsem==1
                E=data(:,j+1);
            end
            figure
           

            %h1 = bar(1,Y(1),.8);hold on
            %h2 = bar(2,Y(2),.8);hold on
            %h3 = bar(3,Y(3),.8);hold on
            %if ncond>3
            %    h4 = bar(4,Y(4),.8);hold on
            %end
            %x1 = get(h1,'xdata');
            %x1m = mean(x1([1 3],:));
            %x2 = get(h2,'xdata');
            %x2m = mean(x2([1 3],:));
            %x3 = get(h3,'xdata');
            %x3m = mean(x3([1 3],:));
            %if ncond>3
            %    x4 = get(h4,'xdata');
            %    x4m = mean(x4([1 3],:));
            %end
            %if ncond > 4
            %    h5 = bar(5,Y(5),.8);hold on
            %    h6 = bar(6,Y(6),.8);hold on
            %    h7 = bar(7,Y(7),.8);hold on
            %    h8 = bar(8,Y(8),.8);hold on
            %    x5 = get(h5,'xdata');
            %    x5m = mean(x5([1 3],:));
            %    x6 = get(h6,'xdata');
            %    x6m = mean(x6([1 3],:));
            %    x7 = get(h7,'xdata');
            %    x7m = mean(x7([1 3],:));
            %    x8 = get(h8,'xdata');
            %    x8m = mean(x8([1 3],:));
            %    if ncond > 8
            %        h9 = bar(9,Y(9),.8);hold on
            %        x9 = get(h9,'xdata');
            %        x9m = mean(x9([1 3],:));
            %    end
            %    if ncond>9
            %        h10 = bar(10,Y(10),.8);hold on
            %        h11 = bar(11,Y(11),.8);hold on
            %        h12 = bar(12,Y(12),.8);hold on
            %        h13 = bar(13,Y(13),.8);hold on
            %        h14 = bar(14,Y(14),.8);hold on
            %        h15 = bar(15,Y(15),.8);hold on
            %        h16 = bar(16,Y(16),.8);hold on
            %        h17 = bar(17,Y(17),.8);hold on
            %        h18 = bar(18,Y(18),.8);hold on
            %        x10 = get(h10,'xdata');
            %        x10m = mean(x10([1 3],:));
            %        x11 = get(h11,'xdata');
            %        x11m = mean(x11([1 3],:));
            %        x12 = get(h12,'xdata');
            %        x12m = mean(x12([1 3],:));
            %        x13 = get(h13,'xdata');
            %        x13m = mean(x13([1 3],:));
            %        x14 = get(h14,'xdata');
            %        x14m = mean(x14([1 3],:));
            %        x15 = get(h15,'xdata');
            %        x15m = mean(x15([1 3],:));
            %        x16 = get(h16,'xdata');
            %        x16m = mean(x16([1 3],:));
            %        x17 = get(h17,'xdata');
            %        x17m = mean(x17([1 3],:));
            %        x18 = get(h18,'xdata');
            %        x18m = mean(x18([1 3],:));
            %    end
            %end
            %if lcsem==1
            %    upper1 = E(1);
            %    upper1(find(Y(1) < 0)) = 0;
            %    lower1 = E(1);
            %    lower1(find(Y(1) >= 0)) = 0;
            %    e1 = errorbar(x1m,Y(1),lower1,upper1,'.k');hold on;
            %    errorbar_tick(e1,110);hold on;
            %    upper2 = E(2);
            %    upper2(find(Y(2) < 0)) = 0;
            %    lower2 = E(2);
            %    lower2(find(Y(2) >= 0)) = 0;
            %    e2 = errorbar(x2m,Y(2),lower2,upper2,'.k');hold on;
            %    errorbar_tick(e2,110);hold on;
            %    upper3 = E(3);
            %    upper3(find(Y(3) < 0)) = 0;
            %    lower3 = E(3);
            %    lower3(find(Y(3) >= 0)) = 0;
            %    e3 = errorbar(x3m,Y(3),lower3,upper3,'.k');hold on;
            %    errorbar_tick(e3,110);
            %    if ncond>3
            %        upper4 = E(4);
            %        upper4(find(Y(4) < 0)) = 0;
            %        lower4 = E(4);
            %        lower4(find(Y(4) >= 0)) = 0;
            %        e4 = errorbar(x4m,Y(4),lower4,upper4,'.k');hold on;
            %        errorbar_tick(e4,110);
            %    end
            %    if ncond > 4
            %        upper5 = E(5);
            %        upper5(find(Y(5) < 0)) = 0;
            %        lower5 = E(5);
            %        lower5(find(Y(5) >= 0)) = 0;
            %        e5 = errorbar(x5m,Y(5),lower5,upper5,'.k');hold on;
            %        errorbar_tick(e5,110);hold on;
            %        upper6 = E(6);
            %        upper6(find(Y(6) < 0)) = 0;
            %        lower6 = E(6);
            %        lower6(find(Y(6) >= 0)) = 0;
            %        e6 = errorbar(x6m,Y(6),lower6,upper6,'.k');hold on;
            %        errorbar_tick(e6,110);hold on;
            %        upper7 = E(7);
            %        upper7(find(Y(7) < 0)) = 0;
            %        lower7 = E(7);
            %        lower7(find(Y(7) >= 0)) = 0;
            %        e7 = errorbar(x7m,Y(7),lower7,upper7,'.k');hold on;
            %        errorbar_tick(e7,110);
            %        upper8 = E(8);
            %        upper8(find(Y(8) < 0)) = 0;
            %        lower8 = E(8);
            %        lower8(find(Y(8) >= 0)) = 0;
            %        e8 = errorbar(x8m,Y(8),lower8,upper8,'.k');hold on;
            %        errorbar_tick(e8,110);
            %        if ncond > 8
            %            upper9 = E(9);
            %            upper9(find(Y(9) < 0)) = 0;
            %            lower9 = E(9);
            %            lower9(find(Y(9) >= 0)) = 0;
            %            e9 = errorbar(x9m,Y(9),lower9,upper9,'.k');hold on;
            %            errorbar_tick(e9,110);
            %        end
            %        if ncond>9
            %            upper10 = E(10);
            %            upper10(find(Y(10) < 0)) = 0;
            %            lower10 = E(10);
            %            lower10(find(Y(10) >= 0)) = 0;
            %            e10 = errorbar(x10m,Y(10),lower10,upper10,'.k');hold on;
            %            errorbar_tick(e10,110);
            %            upper11 = E(11);
            %            upper11(find(Y(11) < 0)) = 0;
            %            lower11 = E(11);
            %            lower11(find(Y(11) >= 0)) = 0;
            %            e11 = errorbar(x11m,Y(11),lower11,upper11,'.k');hold on;
            %            errorbar_tick(e11,110);
            %            upper12 = E(12);
            %            upper12(find(Y(12) < 0)) = 0;
            %            lower12 = E(12);
            %            lower12(find(Y(12) >= 0)) = 0;
            %            e12 = errorbar(x12m,Y(12),lower12,upper12,'.k');hold on;
            %            errorbar_tick(e12,110);
            %            upper13 = E(13);
            %            upper13(find(Y(13) < 0)) = 0;
            %            lower13 = E(13);
            %            lower13(find(Y(13) >= 0)) = 0;
            %            e13 = errorbar(x13m,Y(13),lower13,upper13,'.k');hold on;
            %            errorbar_tick(e13,110);
            %            upper14 = E(14);
            %            upper14(find(Y(14) < 0)) = 0;
            %            lower14 = E(14);
            %            lower14(find(Y(14) >= 0)) = 0;
            %            e14 = errorbar(x14m,Y(14),lower14,upper14,'.k');hold on;
            %            errorbar_tick(e14,110);
            %            upper15 = E(15);
            %            upper15(find(Y(15) < 0)) = 0;
            %            lower15 = E(15);
            %            lower15(find(Y(15) >= 0)) = 0;
            %            e15 = errorbar(x15m,Y(15),lower15,upper15,'.k');hold on;
            %            errorbar_tick(e15,110);
            %            upper16 = E(16);
            %            upper16(find(Y(16) < 0)) = 0;
            %            lower16 = E(16);
            %            lower16(find(Y(16) >= 0)) = 0;
            %            e16 = errorbar(x16m,Y(16),lower16,upper16,'.k');hold on;
            %            errorbar_tick(e16,110);
            %            upper17 = E(17);
            %            upper17(find(Y(17) < 0)) = 0;
            %            lower17 = E(17);
            %            lower17(find(Y(17) >= 0)) = 0;
            %            e17 = errorbar(x17m,Y17,lower17,upper17,'.k');hold on;
            %            errorbar_tick(e17,110);
            %            upper18 = E(18);
            %            upper18(find(Y(18) < 0)) = 0;
            %            lower18 = E(18);
            %            lower18(find(Y(18) >= 0)) = 0;
            %            e18 = errorbar(x18m,Y(18),lower18,upper18,'.k');hold on;
            %            errorbar_tick(e18,110);
            %        end
            %    end
            %START170425
            xm=zeros(1,ncond);
            h1 = bar(1,Y(1),.8);hold on
            h2 = bar(2,Y(2),.8);hold on
            h3 = bar(3,Y(3),.8);hold on
            if ncond>3
                h4 = bar(4,Y(4),.8);hold on
                if ncond>4
                    h5 = bar(5,Y(5),.8);hold on
                    h6 = bar(6,Y(6),.8);hold on
                    h7 = bar(7,Y(7),.8);hold on
                    h8 = bar(8,Y(8),.8);hold on
                    if ncond>8
                        h9 = bar(9,Y(9),.8);hold on
                        if ncond>9
                            h10 = bar(10,Y(10),.8);hold on
                            h11 = bar(11,Y(11),.8);hold on
                            h12 = bar(12,Y(12),.8);hold on
                            h13 = bar(13,Y(13),.8);hold on
                            h14 = bar(14,Y(14),.8);hold on
                            h15 = bar(15,Y(15),.8);hold on
                            h16 = bar(16,Y(16),.8);hold on
                            h17 = bar(17,Y(17),.8);hold on
                            h18 = bar(18,Y(18),.8);hold on
                        end 
                    end 
                end
            end
            x1 = get(h1,'xdata');
            xm(1) = mean(x1([1 3],:));
            x2 = get(h2,'xdata');
            xm(2) = mean(x2([1 3],:));
            x3 = get(h3,'xdata');
            xm(3) = mean(x3([1 3],:));
            if ncond>3
                x4 = get(h4,'xdata');
                xm(4) = mean(x4([1 3],:));
                if ncond>4
                    x5 = get(h5,'xdata');
                    xm(5) = mean(x5([1 3],:));
                    x6 = get(h6,'xdata');
                    xm(6) = mean(x6([1 3],:));
                    x7 = get(h7,'xdata');
                    xm(7) = mean(x7([1 3],:));
                    x8 = get(h8,'xdata');
                    xm(8) = mean(x8([1 3],:));
                    if ncond>8
                        x9 = get(h9,'xdata');
                        xm(9) = mean(x9([1 3],:));
                        if ncond>9
                            x10 = get(h10,'xdata');
                            xm(10) = mean(x10([1 3],:));
                            x11 = get(h11,'xdata');
                            xm(11) = mean(x11([1 3],:));
                            x12 = get(h12,'xdata');
                            xm(12) = mean(x12([1 3],:));
                            x13 = get(h13,'xdata');
                            xm(13) = mean(x13([1 3],:));
                            x14 = get(h14,'xdata');
                            xm(14) = mean(x14([1 3],:));
                            x15 = get(h15,'xdata');
                            xm(15) = mean(x15([1 3],:));
                            x16 = get(h16,'xdata');
                            xm(16) = mean(x16([1 3],:));
                            x17 = get(h17,'xdata');
                            xm(17) = mean(x17([1 3],:));
                            x18 = get(h18,'xdata');
                            xm(18) = mean(x18([1 3],:));
                        end
                    end
                end
            end
            if lcsem==1
                upper1 = E(1);
                upper1(find(Y(1) < 0)) = 0;
                lower1 = E(1);
                lower1(find(Y(1) >= 0)) = 0;
                e1 = errorbar(xm(1),Y(1),lower1,upper1,'.k');hold on;
                errorbar_tick(e1,110);hold on;
                upper2 = E(2);
                upper2(find(Y(2) < 0)) = 0;
                lower2 = E(2);
                lower2(find(Y(2) >= 0)) = 0;
                e2 = errorbar(xm(2),Y(2),lower2,upper2,'.k');hold on;
                errorbar_tick(e2,110);hold on;
                upper3 = E(3);
                upper3(find(Y(3) < 0)) = 0;
                lower3 = E(3);
                lower3(find(Y(3) >= 0)) = 0;
                e3 = errorbar(xm(3),Y(3),lower3,upper3,'.k');hold on;
                errorbar_tick(e3,110);
                if ncond>3
                    upper4 = E(4);
                    upper4(find(Y(4) < 0)) = 0;
                    lower4 = E(4);
                    lower4(find(Y(4) >= 0)) = 0;
                    e4 = errorbar(xm(4),Y(4),lower4,upper4,'.k');hold on;
                    errorbar_tick(e4,110);
                    if ncond > 4
                        upper5 = E(5);
                        upper5(find(Y(5) < 0)) = 0;
                        lower5 = E(5);
                        lower5(find(Y(5) >= 0)) = 0;
                        e5 = errorbar(xm(5),Y(5),lower5,upper5,'.k');hold on;
                        errorbar_tick(e5,110);hold on;
                        upper6 = E(6);
                        upper6(find(Y(6) < 0)) = 0;
                        lower6 = E(6);
                        lower6(find(Y(6) >= 0)) = 0;
                        e6 = errorbar(xm(6),Y(6),lower6,upper6,'.k');hold on;
                        errorbar_tick(e6,110);hold on;
                        upper7 = E(7);
                        upper7(find(Y(7) < 0)) = 0;
                        lower7 = E(7);
                        lower7(find(Y(7) >= 0)) = 0;
                        e7 = errorbar(xm(7),Y(7),lower7,upper7,'.k');hold on;
                        errorbar_tick(e7,110);
                        upper8 = E(8);
                        upper8(find(Y(8) < 0)) = 0;
                        lower8 = E(8);
                        lower8(find(Y(8) >= 0)) = 0;
                        e8 = errorbar(xm(8),Y(8),lower8,upper8,'.k');hold on;
                        errorbar_tick(e8,110);
                        if ncond > 8
                            upper9 = E(9);
                            upper9(find(Y(9) < 0)) = 0;
                            lower9 = E(9);
                            lower9(find(Y(9) >= 0)) = 0;
                            e9 = errorbar(xm(9),Y(9),lower9,upper9,'.k');hold on;
                            errorbar_tick(e9,110);
                            if ncond>9
                                upper10 = E(10);
                                upper10(find(Y(10) < 0)) = 0;
                                lower10 = E(10);
                                lower10(find(Y(10) >= 0)) = 0;
                                e10 = errorbar(xm(10),Y(10),lower10,upper10,'.k');hold on;
                                errorbar_tick(e10,110);
                                upper11 = E(11);
                                upper11(find(Y(11) < 0)) = 0;
                                lower11 = E(11);
                                lower11(find(Y(11) >= 0)) = 0;
                                e11 = errorbar(xm(11),Y(11),lower11,upper11,'.k');hold on;
                                errorbar_tick(e11,110);
                                upper12 = E(12);
                                upper12(find(Y(12) < 0)) = 0;
                                lower12 = E(12);
                                lower12(find(Y(12) >= 0)) = 0;
                                e12 = errorbar(xm(12),Y(12),lower12,upper12,'.k');hold on;
                                errorbar_tick(e12,110);
                                upper13 = E(13);
                                upper13(find(Y(13) < 0)) = 0;
                                lower13 = E(13);
                                lower13(find(Y(13) >= 0)) = 0;
                                e13 = errorbar(xm(13),Y(13),lower13,upper13,'.k');hold on;
                                errorbar_tick(e13,110);
                                upper14 = E(14);
                                upper14(find(Y(14) < 0)) = 0;
                                lower14 = E(14);
                                lower14(find(Y(14) >= 0)) = 0;
                                e14 = errorbar(xm(14),Y(14),lower14,upper14,'.k');hold on;
                                errorbar_tick(e14,110);
                                upper15 = E(15);
                                upper15(find(Y(15) < 0)) = 0;
                                lower15 = E(15);
                                lower15(find(Y(15) >= 0)) = 0;
                                e15 = errorbar(xm(15),Y(15),lower15,upper15,'.k');hold on;
                                errorbar_tick(e15,110);
                                upper16 = E(16);
                                upper16(find(Y(16) < 0)) = 0;
                                lower16 = E(16);
                                lower16(find(Y(16) >= 0)) = 0;
                                e16 = errorbar(xm(16),Y(16),lower16,upper16,'.k');hold on;
                                errorbar_tick(e16,110);
                                upper17 = E(17);
                                upper17(find(Y(17) < 0)) = 0;
                                lower17 = E(17);
                                lower17(find(Y(17) >= 0)) = 0;
                                e17 = errorbar(xm(17),Y17,lower17,upper17,'.k');hold on;
                                errorbar_tick(e17,110);
                                upper18 = E(18);
                                upper18(find(Y(18) < 0)) = 0;
                                lower18 = E(18);
                                lower18(find(Y(18) >= 0)) = 0;
                                e18 = errorbar(xm(18),Y(18),lower18,upper18,'.k');hold on;
                                errorbar_tick(e18,110);
                            end
                        end
                    end
                end





                set(e1,'marker','none','linewidth',1)
                set(e2,'marker','none','linewidth',1)
                set(e3,'marker','none','linewidth',1)
                if ncond>3
                    set(e4,'marker','none','linewidth',1)
                end
                if ncond > 4
                    set(e5,'marker','none','linewidth',1)
                    set(e6,'marker','none','linewidth',1)
                    set(e7,'marker','none','linewidth',1)
                    set(e8,'marker','none','linewidth',1)
                    if ncond > 8
                        set(e9,'marker','none','linewidth',1)
                    end
                    if ncond>9
                        set(e10,'marker','none','linewidth',1)
                        set(e11,'marker','none','linewidth',1)
                        set(e12,'marker','none','linewidth',1)
                        set(e13,'marker','none','linewidth',1)
                        set(e14,'marker','none','linewidth',1)
                        set(e15,'marker','none','linewidth',1)
                        set(e16,'marker','none','linewidth',1)
                        set(e17,'marker','none','linewidth',1)
                        set(e18,'marker','none','linewidth',1)
                    end
                end
            end
            if ncond==3
                set(h1,'facecolor','k','edgecolor','k','linewidth',2)
                set(h2,'facecolor','w','edgecolor','k','linewidth',2)
                set(h3,'facecolor',graydark,'edgecolor','k','linewidth',2)
            elseif ncond==4
                set(h1,'facecolor','k','edgecolor','k','linewidth',2)
                set(h2,'facecolor','w','edgecolor','k','linewidth',2)
                set(h3,'facecolor',graylight,'edgecolor','k','linewidth',2)
                set(h4,'facecolor',graydark,'edgecolor','k','linewidth',2)
            elseif ncond==8
                set(h1,'facecolor','k','edgecolor','k','linewidth',2)
                set(h2,'facecolor','k','edgecolor','k','linewidth',2)
                set(h3,'facecolor','w','edgecolor','k','linewidth',2)
                set(h4,'facecolor','w','edgecolor','k','linewidth',2)
                set(h5,'facecolor',graylight,'edgecolor','k','linewidth',2)
                set(h6,'facecolor',graylight,'edgecolor','k','linewidth',2)
                set(h7,'facecolor',graydark,'edgecolor','k','linewidth',2)
                set(h8,'facecolor',graydark,'edgecolor','k','linewidth',2)
            elseif ncond==9
                set(h1,'facecolor','k','edgecolor','k','linewidth',2)
                set(h2,'facecolor','k','edgecolor','k','linewidth',2)
                set(h3,'facecolor','k','edgecolor','k','linewidth',2)
                set(h4,'facecolor','w','edgecolor','k','linewidth',2)
                set(h5,'facecolor','w','edgecolor','k','linewidth',2)
                set(h6,'facecolor','w','edgecolor','k','linewidth',2)
                set(h7,'facecolor',graydark,'edgecolor','k','linewidth',2)
                set(h8,'facecolor',graydark,'edgecolor','k','linewidth',2)
                set(h9,'facecolor',graydark,'edgecolor','k','linewidth',2)
            elseif ncond==18
                set(h1,'facecolor','k','edgecolor','k','linewidth',2)
                set(h2,'facecolor','k','edgecolor','k','linewidth',2)
                set(h3,'facecolor','k','edgecolor','k','linewidth',2)
                set(h4,'facecolor','k','edgecolor','k','linewidth',2)
                set(h5,'facecolor','k','edgecolor','k','linewidth',2)
                set(h6,'facecolor','k','edgecolor','k','linewidth',2)
                set(h7,'facecolor','w','edgecolor','k','linewidth',2)
                set(h8,'facecolor','w','edgecolor','k','linewidth',2)
                set(h9,'facecolor','w','edgecolor','k','linewidth',2)
                set(h10,'facecolor','w','edgecolor','k','linewidth',2)
                set(h11,'facecolor','w','edgecolor','k','linewidth',2)
                set(h12,'facecolor','w','edgecolor','k','linewidth',2)
                set(h13,'facecolor',graydark,'edgecolor','k','linewidth',2)
                set(h14,'facecolor',graydark,'edgecolor','k','linewidth',2)
                set(h15,'facecolor',graydark,'edgecolor','k','linewidth',2)
                set(h16,'facecolor',graydark,'edgecolor','k','linewidth',2)
                set(h17,'facecolor',graydark,'edgecolor','k','linewidth',2)
                set(h18,'facecolor',graydark,'edgecolor','k','linewidth',2)
            end

            %set(gca,'box','off','linewidth',2,'tickdir','out','xtick',[1:ncond],'xticklabel',[],'xlim',[.5 ncond+.5], ...
            %    'ylim',[-.82 1.05],'ytick',[-.8 -.6 -.4 -.2 0 .2 .4 .6 .8 1],'yticklabel',[])

            %set(gca,'box','off','linewidth',2,'tickdir','out','xtick',[1:ncond],'xticklabel',condstr,'xlim',[.5 ncond+.5],...
            %    'ylim',[mindata-.1 maxdata+.1])
            set(gca,'box','off','linewidth',2,'tickdir','out','xtick',[1:ncond],'xticklabel',condstr,'xlim',[.5 ncond+.5])

            %if i==3|i==29
            %    set(gca,'box','off','linewidth',2,'tickdir','out','xtick',[1:ncond],'xticklabel',condstr,'xlim',[.5 ncond+.5],...
            %        'ylim',[-.14 .33])
            %else
            %    set(gca,'box','off','linewidth',2,'tickdir','out','xtick',[1:ncond],'xticklabel',condstr,'xlim',[.5 ncond+.5],...
            %        'ylim',[-.43 0])
            %end
            %set(gca,'box','off','linewidth',2,'tickdir','out','xtick',[1:ncond],'xticklabel',condstr,'xlim',[.5 ncond+.5],...
            %    'ylim',[-.43 .33])
            set(gca,'box','off','linewidth',2,'tickdir','out','xtick',[1:ncond],'xticklabel',[],'xlim',[.5 ncond+.5],...
                'ylim',[-.43 .33],'yticklabel',[],'ytick',[-.4 -.3 -.2 -.1 0 .1 .2 .3])

            %text(1,1,regnames(i),'Color','black','FontSize',24)
            %title(regnames(i),'Color','black','FontSize',16)
            %y = sprintf('%s %s',subject,char(deblank(regnames(i,:))));
            %title(y,'Color','black','FontSize',16)
            %text(4,.24,nvox(i),'Color','black','FontSize',18)
        else
            Y=data(:,j);
            figure
            s=scatter(freq,Y,36,'k');
            %set(gca,'box','off','linewidth',2,'tickdir','out','xtick',[0:ncond-1],'xticklabel',condstr,...
            %    'xlim',[min(freq)-.5 max(freq)+.5])
            %title(regnames(i),'Color','black','FontSize',16)

            %meanY=zeros(1,ncond);
            %semY=zeros(1,ncond);
            %for j=0:ncond-1
            %    meanY(j+1)=mean(Y(find(freq==j)));
            %    semY(j+1)=std(Y(find(freq==j)))/sqrt(length(Y(find(freq==j))));
            %end
            meanY=zeros(1,ncond);
            semY=zeros(1,ncond);
            %condstr1='';
            condstri=zeros(1,ncond);
            for j=0:ncond-1
                idx=find(freq==j);
                meanY(j+1)=mean(Y(idx));
                semY(j+1)=std(Y(idx))/sqrt(length(Y(idx)));
                %condstr1=strvcat(condstr1,condstr(idx(1))); 
                condstri(j+1)=idx(1); 
            end
            set(gca,'box','off','linewidth',2,'tickdir','out','xtick',[0:ncond-1],'xticklabel',condstr(condstri),...
                'xlim',[min(freq)-.5 max(freq)+.5],'FontSize',16)
            y=sprintf('%s %s',subject,char(deblank(regnames(i,:))));
            title(y,'Color','black','FontSize',16)


            hold on;plot([-.25 .25],[meanY(1) meanY(1)],'-r')
            hold on;plot([.75 1.25],[meanY(2) meanY(2)],'-r')
            hold on;plot([1.75 2.25],[meanY(3) meanY(3)],'-r')
            hold off

            hold on;plot([-.25 .25],[meanY(1)+semY(1) meanY(1)+semY(1)],'--r')
            hold on;plot([-.25 .25],[meanY(1)-semY(1) meanY(1)-semY(1)],'--r')
            hold on;plot([.75 1.25],[meanY(2)+semY(2) meanY(2)+semY(2)],'--r')
            hold on;plot([.75 1.25],[meanY(2)-semY(2) meanY(2)-semY(2)],'--r')
            hold on;plot([1.75 2.25],[meanY(3)+semY(3) meanY(3)+semY(3)],'--r')
            hold on;plot([1.75 2.25],[meanY(3)-semY(3) meanY(3)-semY(3)],'--r')
            hold off
        end

        %text(30,maxy(i)-.05*maxy(i),condstr(index(1,1)),'Color','r','FontSize',16)
        %text(30,maxy(i)-.2*maxy(i),condstr(index(2,1)),'Color','g','FontSize',16)
        y = sprintf('%s %s',subject,char(deblank(regnames(i,:))));
        %print('-dtiff',char(strrep(y,' ','_')),'-r300')
        %print('-dtiff',char(strrep(y,' ','_')))
           %'YLim',[.65 1.15], 'FontSize',16)
        %if i==1 
        %    text(8,1.1,regnames(i),'Color','k','FontSize',32)
        %elseif i==2 | i==4 | i==5
        %    text(8,.7,regnames(i),'Color','k','FontSize',32)
        %elseif i==3 
        %    text(15,.7,regnames(i),'Color','k','FontSize',32)
        %elseif i==6
        %    text(6,1.1,regnames(i),'Color','k','FontSize',32)
        %elseif i==8
        %    text(11,1.1,regnames(i),'Color','k','FontSize',32)
        %else
        %    text(2,1.1,regnames(i),'Color','k','FontSize',32)
        %end

        %text(30,maxy(i)-.05*maxy(i),condstr(index(1,1)),'Color','r','FontSize',16)
        %text(30,maxy(i)-.2*maxy(i),condstr(index(2,1)),'Color','g','FontSize',16)
        y = sprintf('%s %s',subject,char(deblank(regnames(i,:))));
        %print('-dtiff',char(strrep(y,' ','_')),'-r300')
        %print('-dtiff',char(strrep(y,' ','_')))

        %y = sprintf('%s %s',subject_title,char(deblank(regnames(i,:))));
        %print('-dtiff',char(strrep(y,' ','_')),'-r600')
        %print('-depsc2',char(strrep(y,' ','_')),'-r600')
        print('-depsc2',char(strrep(y,' ','_')))
        
    end

  %end

end
