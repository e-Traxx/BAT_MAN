# Python script to generate the DBC file

num_cells = 140
signals_per_mux = 5
num_mux = num_cells // signals_per_mux  # 35

dbc_content = []
dbc_content.append('VERSION "1.0"\n')
dbc_content.append('NS_ :\n    NS_DESC_\n    CM_\n    BA_DEF_\n    BA_\n    VAL_\n    CAT_DEF_\n    CAT_\n    FILTER\n    BA_DEF_DEF_\n    EV_DATA_\n    ENVVAR_DATA_\n    SGTYPE_\n    SGTYPE_VAL_\n    BA_DEF_SGTYPE_\n    BA_SGTYPE_\n    SIG_TYPE_REF_\n    VAL_TABLE_\n    SIG_GROUP_\n    SIG_VALTYPE_\n    SIGTYPE_VALTYPE_\n\n')
dbc_content.append('BS_:\n\n')
dbc_content.append('BU_: ECU\n\n')
dbc_content.append('BO_ 256 individual_messages: 8 ECU\n')
dbc_content.append(' SG_ Mux : 0|8@1+ (1,0) [0|34] "" Vector__XXX\n\n')

for mux in range(1, num_mux + 1):
    start_cell = (mux - 1) * signals_per_mux + 1
    for i in range(signals_per_mux):
        cell_num = start_cell + i
        start_bit = 8 + i * 10
        dbc_content.append(f' SG_ cell_voltage_{cell_num} m{mux} : {start_bit}|10@1+ (1,0) [0|1024] "V" ECU\n')
    dbc_content.append('\n')

# Write to DBC file
with open('individual_messages_10bit.dbc', 'w') as file:
    file.writelines(dbc_content)

print("DBC file 'individual_messages.dbc' has been generated successfully.")
