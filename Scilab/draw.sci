// ============================================================
// Scilab script: Read serial COM port, log data, plot realtime
// ============================================================
atomsInstall("serial");
atomsLoad("serial");
clc;
clear;
clf();

com_port = 4; // COM4
baud_cfg = "115200,n,8,1";

log_path = "D:\Embedded\Scilab\data_log.txt";

try
    mclose(log_file);
catch
end

log_file = mopen(log_path, "wt");
if log_file == -1 then
    error("Can not open log file");
end

printf("Open COM%d...\n", com_port);
h = openserial(com_port, baud_cfg);
sleep(2000);




line = ""; 
line = readserial(h);
//disp(line);

clf();
function val = extract_value(str, label)
    pos = strindex(str, label + " = ");
    if pos <> [] then
        // take "label = "
        sub = part(str, pos(1) + length(label + " = "):$);

 
        semi_pos = strindex(sub, ";");
        if semi_pos <> [] then
            sub = part(sub, 1:semi_pos(1)-1); // get front ;;
        end

        // take number
        space_pos = strindex(sub, " ");
        if space_pos <> [] then
            sub = part(sub, 1:space_pos(1)-1);
        end

        //disp(sub); 
        val = evstr(sub);
    else
        val = %nan;
        disp("loi");
    end
endfunction

n_max = 1000; 
t = zeros(1, n_max);
pitch_data = zeros(1, n_max);
mea_pitch_data = zeros(1, n_max);
yaw_data = zeros(1, n_max);
mea_yaw_data = zeros(1, n_max);
roll_data = zeros(1, n_max);
mea_roll_data = zeros(1, n_max);
buffer = "";
idx = 0; 

subplot(3,1,1);
ax1 = gca();
p1 = plot(1,1,'b-');
p2 = plot(1,1,'g-');
legend(["Pitch","Mea_Pitch"], "in_upper_left");
xtitle("Pitch"); xgrid;

subplot(3,1,2);
ax2 = gca();
p3 = plot(1,1,'b-');
p4 = plot(1,1,'g-');
legend(["Yaw","Mea_Yaw"], "in_upper_left");
xtitle("Yaw"); xgrid;

subplot(3,1,3);
ax3 = gca();
p5 = plot(1,1,'b-');
p6 = plot(1,1,'g-');
legend(["Roll","Mea_Roll"], "in_upper_left");
xtitle("Roll"); xgrid;

while %t
    chunk = readserial(h);
    if chunk <> "" then
        buffer = buffer + chunk;

        pos = strindex(buffer, "#");
        while pos <> [] do
            line = part(buffer, 1:pos(1)-1);
            
            buffer = part(buffer, pos(1)+1:$);
            D = getdate();
            timestamp = sprintf("%d-%02d-%02d %02d:%02d:%06.3f", D(1), D(2), D(6:8), D(9)+D(10)/1000);
            mfprintf(log_file, "[%s] %s\n##############################################################\n" , timestamp, line); 
            // Parse dữ liệu
            mea_pitch_val = extract_value(line, "mea_pitch");
            pitch_val     = extract_value(line, "pitch_update");
            mea_yaw_val   = extract_value(line, "mea_yaw");
            yaw_val       = extract_value(line, "yaw_update");
            mea_roll_val  = extract_value(line, "mea_roll");
            roll_val      = extract_value(line, "roll_update");
            
            if idx >= 998 then
                ++n_max;   
            end
            // Append dữ liệu
            if ~isnan(mea_pitch_val) & ~isnan(pitch_val) then
                idx = idx + 1;
                mea_pitch_data(idx) = mea_pitch_val;
                pitch_data(idx)     = pitch_val;
            end
            if ~isnan(mea_yaw_val) & ~isnan(yaw_val) then
                mea_yaw_data(idx) = mea_yaw_val;
                yaw_data(idx)     = yaw_val;
            end
            if ~isnan(mea_roll_val) & ~isnan(roll_val) then
                mea_roll_data(idx) = mea_roll_val;
                roll_data(idx)     = roll_val;
            end

            // Vẽ
            t = 1:idx;
      
            
            // ================= Pitch =================
            /*subplot(3,1,1);
            plot(t, pitch_data(1:idx), 'b-o'); 
            plot(t, mea_pitch_data(1:idx), 'g-o');    
            legend(["Pitch","Mea_Pitch"], "in_upper_left");
            xtitle("Pitch");
            xgrid;
            
            subplot(3,1,2);
            plot(t, yaw_data(1:idx), 'b-o');      
            plot(t, mea_yaw_data(1:idx), 'g-o');    
            legend(["Yaw","Mea_Yaw"], "in_upper_left");
            xtitle("Yaw"); 
            xgrid;
            
            subplot(3,1,3);
            plot(t, roll_data(1:idx), 'b-o');      
            plot(t, mea_roll_data(1:idx), 'g-o');    
            legend(["Roll","Mea_Roll"], "in_upper_left");
            xtitle("Roll"); 
            xgrid;*/
            
            p1.data = [t' pitch_data(1:idx)'];
            p2.data = [t' mea_pitch_data(1:idx)'];
            p3.data = [t' yaw_data(1:idx)'];
            p4.data = [t' mea_yaw_data(1:idx)'];
            p5.data = [t' roll_data(1:idx)'];
            p6.data = [t' mea_roll_data(1:idx)'];
            replot([ax1 ax2 ax3]);
            
            pos = strindex(buffer, "#");
            sleep(50);
        end
    end
    sleep(50);
end



mclose(log_file);
closeserial(h);
