# idf_component_register(SRCS "Batman_esp.c" "can_handler.c" "diagnostic_handler.c" "spi_handler.c" "Robin" "OTA" "WIFI.c" "Albert" "Diag"
#                     INCLUDE_DIRS "include")

idf_component_register(
    SRCS "Batman_esp.c" 
         "can_handler.c" 
         "diagnostic_handler.c" 
         "spi_handler.c" 
         "WIFI.c"
         "Albert/Albert.c" 
         "Robin/Robin_balancing.c" 
         "Robin/Robin_handler.c" 
         "Robin/Robin_parser.c" 
         "OTA/Ota_handler.c"
         # If there are any source files in the Diag directory, add them here.
    INCLUDE_DIRS "include"
)
