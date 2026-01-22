keywords = ["RRAM", "STT-MRAM", "PCRAM", "2D_DRAM", "3D_DRAM"]

with open("nvmain_report", "r") as file:
    data_dict = {}
    current_keyword = None

    for line in file:
        for keyword in keywords:
            if keyword in line:
                current_keyword = keyword
                break

        if "totalPower" in line and current_keyword is not None:
            data = line.split("totalPower")[1][:20].strip().rstrip("W")
            if current_keyword in data_dict:
                data_dict[current_keyword].append(data)
            else:
                data_dict[current_keyword] = [data]

# for keyword, data_list in data_dict.items():
#     print((keyword + ' power lists:').rjust(20), data_list)

# 计算每个关键字下的数据总和
total_power_dict = {}
for keyword, data_list in data_dict.items():
    total_power = sum(float(data.rstrip("W")) for data in data_list)
    total_power_dict[keyword] = total_power

# 输出每个关键字下的数据总和
for keyword, total_power in total_power_dict.items():
    keywords = ["RRAM", "STT-MRAM", "PCRAM", "2D_DRAM", "3D_DRAM"]


for keyword, total_power in total_power_dict.items():
    # print((keyword + ' total power:').rjust(20) + " {:.8f} W".format(total_power))
    print((keyword + ' total power:').rjust(22), " {:>10.5f} mW".format(total_power*1000))
