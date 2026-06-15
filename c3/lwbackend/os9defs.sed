# os9defs.sed - rewrite OS-9 system-call NAMES to their literal codes before
# the c3lw.awk filter runs.  The library .a sources write `os9 I$Write` etc.;
# the `os9` directive needs the numeric code at assembly time, and pulling the
# names in as symbols would tangle with the filter's extern/export logic and
# duplicate across objects.  Substituting the literal value sidesteps all that.
#
# Longer names are listed before their prefixes (I$ReadLn before I$Read) so the
# first match wins.  Standard NitrOS-9/OS-9 codes (see lib/os9defs.a).

# ---- function service requests (F$) ----
s/F\$Link/\$00/g
s/F\$Load/\$01/g
s/F\$UnLink/\$02/g
s/F\$Fork/\$03/g
s/F\$Wait/\$04/g
s/F\$Chain/\$05/g
s/F\$Exit/\$06/g
s/F\$Mem/\$07/g
s/F\$Send/\$08/g
s/F\$Icpt/\$09/g
s/F\$Sleep/\$0A/g
s/F\$ID/\$0C/g
s/F\$SPrior/\$0D/g
s/F\$PErr/\$0F/g
s/F\$PrsNam/\$10/g
s/F\$CmpNam/\$11/g
s/F\$STime/\$16/g
s/F\$Time/\$15/g
s/F\$SUser/\$1C/g
s/F\$ID/\$0C/g

# ---- I/O service requests (I$) ----  longer prefixes first
s/I\$ReadLn/\$8B/g
s/I\$WritLn/\$8C/g
s/I\$GetStt/\$8D/g
s/I\$SetStt/\$8E/g
s/I\$MakDir/\$85/g
s/I\$ChgDir/\$86/g
s/I\$DeletX/\$90/g
s/I\$Attach/\$80/g
s/I\$Detach/\$81/g
s/I\$Create/\$83/g
s/I\$Delete/\$87/g
s/I\$Close/\$8F/g
s/I\$Open/\$84/g
s/I\$Seek/\$88/g
s/I\$Read/\$89/g
s/I\$Write/\$8A/g
s/I\$Dup/\$82/g
s/F\$CRC/\$17/g

# ---- error codes (E$), counted from E$PthFul=200 ($C8) in lib/os9defs.a ----
s/E\$SeekRg/\$D2/g
s/E\$EOF/\$D3/g
s/E\$FNA/\$D6/g
s/E\$BPNam/\$D7/g
s/E\$PNNF/\$D8/g
s/E\$CEF/\$DA/g
s/E\$MNF/\$DD/g
s/E\$Read/\$F4/g
s/E\$Write/\$F5/g
s/E\$NotRdy/\$F6/g
s/E\$Seek/\$F7/g
