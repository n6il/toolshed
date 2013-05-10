#!/usr/bin/perl

# This script will "fixup" the BASIC source copy and pasted from the Unravvelled PDFs.
# It wont make it compiliable, but a lot of the work will be done.

LINE:
 while (<>)
 {
      if( substr($_,0,1) eq "*" )
      {
         print $_;
         next;
      }
      elsif ($_ =~ /(^L[0-9A-F][0-9A-F][0-9A-F][0-9A-F]) (\S+?) (\S+?) (.*)$/)
      {
         $_ =~ s/(^L[0-9A-F][0-9A-F][0-9A-F][0-9A-F]) (\S+?) (\S+?) (.*)$/\1\t\2\t\3\t\4/;
      }
      elsif ($_ =~ /(^L[0-9A-F][0-9A-F][0-9A-F][0-9A-F]) (\S+?) (.*)$/)
      {
         $_ =~ s/(^L[0-9A-F][0-9A-F][0-9A-F][0-9A-F]) (\S+?) (.*)$/\1\t\2\t\3/;
      }
      elsif ($_ =~ /(^L[0-9A-F][0-9A-F][0-9A-F][0-9A-F]) (.*)$/)
      {
         $_ =~ s/(^L[0-9A-F][0-9A-F][0-9A-F][0-9A-F]) (.*)$/\1\t\2/;
      }
      elsif ($_ =~ /(^\S+?) (\S+?) (.*)$/)
      {
         $_ =~ s/(^\S+?) (\S+?) (.*)$/\t\1\t\2\t\3/;
      }
      elsif ($_ =~ /(^\S+?) (.*)$/)
      {
         $_ =~ s/(^\S+?) (.*)$/\t\1\t\2/;
      }

# Equate fixup

      $_ =~ s/^\t(.*EQU\t)(\S+) +(.*)/\1\2\t\3/;
      $_ =~ s/^\t(.*EQU\t)(.*)/\1\2/;
      
# Inhert mode fixups

      $_ =~ s/^(.*COM[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*INC[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*DEC[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*ASL[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*ASR[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*LSR[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*LSL[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*ROR[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*ROL[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*NEG[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*TST[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*ROR[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*ROL[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*NEG[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*TST[AB])\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*MUL)\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*ABX)\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*NOP)\t(\S+)\t(.*)$/\1\t\t\2 \3/;
      $_ =~ s/^(.*SEX)\t(\S+)\t(.*)$/\1\t\t\2 \3/;
 
      $_ =~ s/^(RTS.*)/\t\1/;
      $_ =~ s/^(RTI.*)/\t\1/;
      
      print $_;
}
