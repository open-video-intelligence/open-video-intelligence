SET(PKG_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.pc")
SET(INI_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.ini")

# configure pkg-config file
CONFIGURE_FILE("ovi.pc.in" ${PKG_CONFIG} @ONLY)

# install pkg-config file
INSTALL(
    FILES ${PKG_CONFIG}
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
# configure ini file
CONFIGURE_FILE("ovi.ini.in" ${INI_CONFIG} @ONLY)

# install ini file
INSTALL(
    FILES ${INI_CONFIG}
    DESTINATION ${CMAKE_INSTALL_FULL_SYSCONFDIR}/${FW_NAME}
)