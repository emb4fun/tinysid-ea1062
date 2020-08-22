
The latest J-Link software (v6.80b) have some problems programming the external
flash of the iMXRT1062. The configuration is missing in the Jlink Device.xml file.


Therefore you must add the following lines to the JLink Device.xml:


  <!--                 -->
  <!-- NXP (iMXRT106x) -->
  <!--                 -->
  <Device>
    <ChipInfo Vendor="NXP" Name="MIMXRT1062CVL5A" WorkRAMAddr="0x20000000" WorkRAMSize="0x00080000" Core="JLINK_CORE_CORTEX_M7" JLinkScriptFile="Devices/NXP/iMXRT105x/NXP_iMXRT105x.pex" Aliases="MIMXRT1062CVL5A" />
    <FlashBankInfo Name="QSPI Flash" BaseAddr="0x60000000" MaxSize="0x04000000" Loader="Devices/NXP/iMXRT105x/NXP_iMXRT105x_QSPI.elf" LoaderType="FLASH_ALGO_TYPE_OPEN" />
  </Device>
