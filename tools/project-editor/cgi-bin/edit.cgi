                            >          >  �   	  ��         ��  ��  �  4�LargeAnimation33.gif       edit.cgii   APPLaplt! ����     APPLaplt! ����                  ��}�      �            %%   4�CVS                              5%   4�DialogCompIcon_mo.gif            6%   4�DialogNavIcon.gif           
X� @        |This CGI script will be used to perform the edits to the given project. Using "parse CGI" OSAX to break apart the post_args.          
                       . "�� . "��      �FasdUAS 1.101.10   ��   ��    k             l      �� ��   �� * * The contents of this file are subject to the Netscape Public License * Version 1.0 (the "NPL"); you may not use this file except in * compliance with the NPL.  You may obtain a copy of the NPL at * http://www.mozilla.org/NPL/ * * Software distributed under the NPL is distributed on an "AS IS" basis, * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL * for the specific language governing rights and limitations under the * NPL. * * The Initial Developer of this code under the NPL is Netscape * Communications Corporation.  Portions created by Netscape are * Copyright (C) 1998 Netscape Communications Corporation.  All Rights * Reserved.        	  l     ������  ��   	  
  
 l     �� ��     	 edit.cgi         l     ������  ��        l     �� ��    ^ X A very basic CGI example. Use this script as a skeleton for creating more sophisticated         l     �� ��    O I CGIs. All this one does is echo back all the CGI variables passed to it.         l     �� ��    X R As with all AppleScript CGIs, make sure you save this as an application, with the         l     �� ��    D > "Stay Open" and "Never Show Startup Screen" options selected.         l     ������  ��        l     �� ��    J D This file is part of the WebCenter distribution. You are free to do        !   l     �� "��   " !  whatever you wish with it.    !  # $ # l     ������  ��   $  % & % l     �� '��   ' @ : Please report bugs to me, Chris Hawk, chris@slaphappy.com    &  ( ) ( l     ������  ��   )  * + * l     �� ,��   , : 4 Change this to point to the Camelot root directory.    +  - . - j     �� /�� 0 gcamelotroot gCamelotRoot / m      0 0  Ringo:Camelot:    .  1 2 1 j    �� 3�� 0 gsourcetree gSourceTree 3 b     4 5 4 o    ���� 0 gcamelotroot gCamelotRoot 5 m     6 6  tree:    2  7 8 7 j    �� 9�� 0 gsessionpath gSessionPath 9 b     : ; : o    	���� 0 gcamelotroot gCamelotRoot ; m   	 
 < <  Raptor.mcvs    8  = > = l     ������  ��   >  ? @ ? l     �� A��   A R L Create the standard HTTP header. We'll prepend this to any result we return    @  B C B j    �� D�� 0 crlf   D b     E F E l    G�� G I   �� H��
�� .sysontocTEXT       shor H m    ���� ��  ��   F l    I�� I I   �� J��
�� .sysontocTEXT       shor J m    ���� 
��  ��   C  K L K j    ,�� M�� 0 http_header   M b    + N O N b    ) P Q P b    ' R S R b    % T U T b    # V W V b    ! X Y X b     Z [ Z b     \ ] \ m     ^ ^  HTTP/1.0 200 OK    ] o    ���� 0 crlf   [ m     _ _  Server: QuidProQuo    Y o     ���� 0 crlf   W l 	 ! " `�� ` m   ! " a a  MIME-Version: 1.0   ��   U o   # $���� 0 crlf   S m   % & b b  Content-type: text/html    Q o   ' (���� 0 crlf   O o   ) *���� 0 crlf   L  c d c l     ������  ��   d  e f e l      �� g��   g �	This is the AppleEvent handler. After this CGI is launched, is spins its wheels waiting for	an AppleEvent to get here. When it does, this next section of code is executed. After it returns	its result, it goes back to waiting for another AppleEvent.     f  h i h l     ������  ��   i  j k j i   - 0 l m l I     �� n o
�� .WWW�sdochtml        TEXT n l 
     p�� p o      ���� 0 	path_args  ��   o �� q r
�� 
kfor q l 
     s�� s o      ���� 0 http_search_args  ��   r �� t u
�� 
post t l 
     v�� v o      ���� 0 	post_args  ��   u �� w x
�� 
meth w l 
     y�� y o      ���� 0 access_method  ��   x �� z {
�� 
addr z l 
     |�� | o      ���� 0 client_address  ��   { �� } ~
�� 
user } l 
     ��  o      ���� 0 	user_name  ��   ~ �� � �
�� 
pass � l 
     ��� � o      ���� 0 using_password  ��   � �� � �
�� 
frmu � l 
     ��� � o      ���� 0 	from_user  ��   � �� � �
�� 
svnm � l 
     ��� � o      ���� 0 server_name  ��   � �� � �
�� 
svpt � l 
     ��� � o      ���� 0 server_port  ��   � �� � �
�� 
scnm � l 
     ��� � o      ���� 0 script_name  ��   � �� � �
�� 
ctyp � l 
     ��� � o      ���� 0 content_type  ��   � �� � �
�� 
refr � l 
     ��� � o      ���� 0 referrer  ��   � �� ���
�� 
Agnt � o      ���� 0 
user_agent  ��   m k    1 � �  � � � l     ������  ��   �  � � � l     �� ���   � G A This example extracts all the possible variables passed to CGIs.    �  � � � l     �� ���   � = 7 You probably won't need all of these in a typical CGI,    �  � � � l     �� ���   � . ( but it doesn't really hurt to get them.    �  � � � l     ������  ��   �  ��� � Q    1 � � � � k   	 � �  � � � l   �� ���   �   Debug the arguments.    �  � � � l   �� ���   � < 6 return http_header & "<H2>Post Args</H2>" & post_args    �  � � � l   ������  ��   �  � � � l   �� ���   � / ) use CGI parser to break apart post_args.    �  � � � r    
 � � � l    ��� � I   �� ���
�� .PCGIPCGIlist        TEXT � o    ���� 0 	post_args  ��  ��   � o      ���� 0 formdata formData �  � � � l   ������  ��   �  � � � r     � � � l    ��� � I   �� � �
�� .CGIsCGI_****        TEXT � m     � �  
formAction    � �� ���
�� 
OFls � o    ���� 0 formdata formData��  ��   � o      ���� 0 
formaction 
formAction �  � � � r    " � � � l    ��� � I   �� � �
�� .CGIsCGI_****        TEXT � m     � �  cvsUser    � �� � �
�� 
OFls � o    ���� 0 formdata formData � �� ���
�� 
defV � m     � �      ��  ��   � o      ���� 0 cvsuser cvsUser �  � � � r   # 0 � � � l  # , ��� � I  # ,�� � �
�� .CGIsCGI_****        TEXT � m   # $ � �  cvsPassword    � �� � �
�� 
OFls � o   % &�� 0 formdata formData � �~ ��}
�~ 
defV � m   ' ( � �      �}  ��   � o      �|�| 0 cvspassword cvsPassword �  � � � r   1 > � � � l  1 : ��{ � I  1 :�z � �
�z .CGIsCGI_****        TEXT � m   1 2 � �  projectToEdit    � �y � �
�y 
OFls � o   3 4�x�x 0 formdata formData � �w ��v
�w 
defV � m   5 6 � �      �v  �{   � o      �u�u 0 projecttoedit projectToEdit �  � � � r   ? L � � � l  ? H ��t � I  ? H�s � �
�s .CGIsCGI_****        TEXT � m   ? @ � �  
filesToAdd    � �r � �
�r 
OFls � o   A B�q�q 0 formdata formData � �p ��o
�p 
defV � m   C D � �      �o  �t   � o      �n�n 0 
filestoadd 
filesToAdd �  � � � r   M Z � � � l  M V ��m � I  M V�l � �
�l .CGIsCGI_****        TEXT � m   M N � �  filesToRemove    � �k � �
�k 
OFls � o   O P�j�j 0 formdata formData � �i ��h
�i 
defV � m   Q R        �h  �m   � o      �g�g 0 filestoremove filesToRemove �  l  [ [�f�e�f  �e    l  [ [�d�d     validate parameters.     Z   [ o	�c�b l  [ b
�a
 =  [ b o   [ ^�`�` 0 cvsuser cvsUser m   ^ a      �a  	 R   e k�_�^
�_ .ascrerr ****      � **** m   g j + %Please specify a valid CVS user name.   �^  �c  �b    l  p p�]�\�]  �\    Z   p ��[�Z l  p w�Y =  p w o   p s�X�X 0 cvspassword cvsPassword m   s v      �Y   R   z ��W�V
�W .ascrerr ****      � **** m   |  * $Please specify a valid CVS password.   �V  �[  �Z    l  � ��U�T�U  �T    Z   � � !�S�R  l  � �"�Q" =  � �#$# o   � ��P�P 0 projecttoedit projectToEdit$ m   � �%%      �Q  ! R   � ��O&�N
�O .ascrerr ****      � ****& m   � �'' . (Please specify a valid Mac project name.   �N  �S  �R   ()( l  � ��M�L�M  �L  ) *�K* Z   �	+,�J-+ l  � �.�I. =  � �/0/ o   � ��H�H 0 
formaction 
formAction0 m   � �11  Query   �I  , L   � �22 b   � �343 b   � �565 o   � ��G�G 0 http_header  6 m   � �77 $ <H2>Project Query Results</H2>   4 l 	 � �8�F8 I   � ��E9�D�E 0 query_project  9 :;: o   � ��C�C 0 cvsuser cvsUser; <=< o   � ��B�B 0 cvspassword cvsPassword= >�A> o   � ��@�@ 0 projecttoedit projectToEdit�A  �D  �F  �J  - k   �	?? @A@ r   � �BCB J   � �DD E�?E o   � ��>�> 0 crlf  �?  C n     FGF 1   � ��=
�= 
txdlG 1   � ��<
�< 
ascrA HIH r   � �JKJ I   � ��;L�:�; 0 tokenize  L M�9M o   � ��8�8 0 
filestoadd 
filesToAdd�9  �:  K o      �7�7  0 filestoaddlist filesToAddListI NON r   � �PQP I   � ��6R�5�6 0 tokenize  R S�4S o   � ��3�3 0 filestoremove filesToRemove�4  �5  Q o      �2�2 &0 filestoremovelist filesToRemoveListO TUT l  � ��1�0�1  �0  U V�/V L   �	WW b   �XYX b   � �Z[Z o   � ��.�. 0 http_header  [ m   � �\\ % <H2>Project Editor Results</H2>   Y l 	 �]�-] I   ��,^�+�, 0 edit_project  ^ _`_ o   � ��*�* 0 cvsuser cvsUser` aba o   � ��)�) 0 cvspassword cvsPasswordb cdc o   � ��(�( 0 projecttoedit projectToEditd efe o   � ��'�'  0 filestoaddlist filesToAddListf g�&g o   ��%�% &0 filestoremovelist filesToRemoveList�&  �+  �-  �/  �K   � R      �$h�#
�$ .ascrerr ****      � ****h o      �"�" 0 error_message  �#   � k  1ii jkj l �!l�!  l ) # last ditch attempt, logout of cvs.   k mnm Q  "op� o I  ���� 0 
cvs_logout  �  �  p R      ���
� .ascrerr ****      � ****�  �  �   n qrq l ##�s�  s W Q We're not doing much here, so there shouldn't be any errors, but just in case...   r t�t L  #1uu b  #0vwv b  #,xyx o  #(�� 0 http_header  y m  (+zz # <H2>Project Editor Error</H2>   w o  ,/�� 0 error_message  �  ��   k {|{ l     ���  �  | }~} i   1 4� I      ���� 0 query_project  � ��� o      �� 0 cvsuser cvsUser� ��� o      �� 0 cvspassword cvsPassword� ��� o      ��  0 projecttoquery projectToQuery�  �  � k     ��� ��� l     ���  �   "login" to CVS.   � ��� I     ���� 0 	cvs_login  � ��� o    �
�
 0 cvsuser cvsUser� ��	� o    �� 0 cvspassword cvsPassword�	  �  � ��� l   ���  �  � ��� l   ���  � A ; make sure we have latest versions of all referenced files.   � ��� I    ���� 0 checkout_files  � ��� J   	 �� ��� o   	 
� �   0 projecttoquery projectToQuery�  �  �  � ��� l   ������  ��  � ��� l   �����  �   "logout" of cvs.   � ��� I    �������� 0 
cvs_logout  ��  ��  � ��� l   ������  ��  � ��� l   �����  � 8 2 make sure the project is in fact a project file.	   � ��� I    ������� 0 validate_project  � ���� o    ����  0 projecttoquery projectToQuery��  ��  � ��� l   ������  ��  � ��� l   �����  � 7 1 used to convert Mac paths to mozilla tree paths.   � ��� r    +��� l   )���� [    )��� m    ���� � l   (���� I   (�����
�� .corecnte****       ****� o    $���� 0 gsourcetree gSourceTree��  ��  ��  � o      ���� .0 mozillatreepathoffset mozillaTreePathOffset� ��� l  , ,������  ��  � ��� r   , 3��� b   , 1��� b   , /��� m   , -��  <H3>Project:    � o   - .����  0 projecttoquery projectToQuery� m   / 0��  </H3>   � o      ���� 0 query_results  � ��� l  4 4������  ��  � ��� l  4 4�����  �   perform the query.   � ��� r   4 D��� l  4 B���� b   4 B��� o   4 9���� 0 gsourcetree gSourceTree� I   9 A������� 0 replace  � ��� o   : ;����  0 projecttoquery projectToQuery� ��� m   ; <��  /   � ���� m   < =��  :   ��  ��  ��  � o      ���� 0 projectpath projectPath� ��� I   E K������� 0 openproject openProject� ���� o   F G���� 0 projectpath projectPath��  ��  � ��� r   L S��� I   L Q�������� 0 
gettargets 
getTargets��  ��  � o      ���� 0 targetslist targetsList� ��� r   T Y��� n   T W��� o   U W���� 	0 names  � o   T U���� 0 targetslist targetsList� o      ���� 0 targetnames targetNames� ��� X   Z ������ k   j ��� ��� r   j o��� l  j m���� n   j m��� 1   k m��
�� 
pcnt� o   j k���� 0 targetnameref targetNameRef��  � o      ���� 0 
targetname 
targetName� ��� r   p }��� b   p {��� b   p w��� b   p u��� o   p q���� 0 query_results  � m   q t�� ! <H3>Source files in target    � o   u v���� 0 
targetname 
targetName� m   w z��  :</H3>   � o      ���� 0 query_results  �    I   ~ ������� $0 setcurrenttarget setCurrentTarget �� o    ����� 0 
targetname 
targetName��  ��    r   � � I   � �������  0 gettargetfiles getTargetFiles 	��	 o   � ����� 0 
targetname 
targetName��  ��   o      ���� 0 targetfiles targetFiles 
��
 X   � ��� k   � �  l  � �����   C = only store the path name relative to the source tree itself.     r   � � I   � ������� 0 	substring    n   � � 1   � ���
�� 
pcnt o   � ����� 0 
targetfile 
targetFile �� o   � ����� .0 mozillatreepathoffset mozillaTreePathOffset��  ��   o      ����  0 targetfilepath targetFilePath �� r   � � b   � � o   � ����� 0 query_results   l  � � ��  b   � �!"! I   � ���#���� 0 replace  # $%$ o   � �����  0 targetfilepath targetFilePath% &'& m   � �((  :   ' )��) m   � �**  /   ��  ��  " m   � �++ 
 <BR>   ��   o      ���� 0 query_results  ��  �� 0 
targetfile 
targetFile o   � ����� 0 targetfiles targetFiles��  �� 0 targetnameref targetNameRef� o   ] ^���� 0 targetnames targetNames� ,-, I   � ���.���� 0 closeproject closeProject. /��/ o   � �����  0 projecttoquery projectToQuery��  ��  - 010 l  � �������  ��  1 2��2 L   � �33 o   � ����� 0 query_results  ��  ~ 454 l     ������  ��  5 676 i   5 8898 I      ��:���� 0 edit_project  : ;<; o      ���� 0 cvsuser cvsUser< =>= o      ���� 0 cvspassword cvsPassword> ?@? o      ���� 0 projecttoedit projectToEdit@ ABA o      ����  0 filestoaddlist filesToAddListB C��C o      ���� &0 filestoremovelist filesToRemoveList��  ��  9 k     �DD EFE Z     GH����G F     IJI l    K��K =     LML o     ����  0 filestoaddlist filesToAddListM J    ����  ��  J l   N��N =    OPO o    ���� &0 filestoremovelist filesToRemoveListP J    
����  ��  H L    QQ b    RSR b    TUT m    VV - 'No files added or removed from project    U o    ���� 0 projecttoedit projectToEditS m    WW  .   ��  ��  F XYX l   ������  ��  Y Z[Z l   ��\��  \   "login" to CVS.   [ ]^] I    "��_���� 0 	cvs_login  _ `a` o    ���� 0 cvsuser cvsUsera b��b o    ���� 0 cvspassword cvsPassword��  ��  ^ cdc l  # #������  ��  d efe l  # #��g��  g A ; make sure we have latest versions of all referenced files.   f hih r   # ,jkj b   # *lml b   # (non J   # &pp q��q o   # $���� 0 projecttoedit projectToEdit��  o o   & '����  0 filestoaddlist filesToAddListm o   ( )���� &0 filestoremovelist filesToRemoveListk o      ���� "0 filestocheckout filesToCheckouti rsr I   - 3�t�~� 0 checkout_files  t u�}u o   . /�|�| "0 filestocheckout filesToCheckout�}  �~  s vwv l  4 4�{�z�{  �z  w xyx l  4 4�yz�y  z 8 2 make sure the project is in fact a project file.	   y {|{ I   4 :�x}�w�x 0 validate_project  } ~�v~ o   5 6�u�u 0 projecttoedit projectToEdit�v  �w  | � l  ; ;�t�s�t  �s  � ��� l  ; ;�r��r  �   perform the edits.   � ��� I   ; A�q��p�q 0 modify_read_only  � ��o� o   < =�n�n 0 projecttoedit projectToEdit�o  �p  � ��� Z  B T���m�l� l  B F��k� >   B F��� o   B C�j�j  0 filestoaddlist filesToAddList� J   C E�i�i  �k  � I   I P�h��g�h 0 	add_files  � ��� o   J K�f�f 0 projecttoedit projectToEdit� ��e� o   K L�d�d  0 filestoaddlist filesToAddList�e  �g  �m  �l  � ��� Z  U g���c�b� l  U Y��a� >   U Y��� o   U V�`�` &0 filestoremovelist filesToRemoveList� J   V X�_�_  �a  � I   \ c�^��]�^ 0 remove_files  � ��� o   ] ^�\�\ 0 projecttoedit projectToEdit� ��[� o   ^ _�Z�Z &0 filestoremovelist filesToRemoveList�[  �]  �c  �b  � ��� l  h h�Y�X�Y  �X  � ��� l  h h�W��W  � K E finally, check in the project, with a comment stating what was done.   � ��� r   h q��� I   h o�V��U�V 0 summarize_edits  � ��� o   i j�T�T  0 filestoaddlist filesToAddList� ��S� o   j k�R�R &0 filestoremovelist filesToRemoveList�S  �U  � o      �Q�Q  0 checkincomment checkInComment� ��� l  r r�P��P  � C = if (projectToEdit starts with "mozilla/build/mac/test") then   � ��� I   r {�O��N�O 0 checkin_files  � ��� J   s v�� ��M� o   s t�L�L 0 projecttoedit projectToEdit�M  � ��K� o   v w�J�J  0 checkincomment checkInComment�K  �N  � ��� l  | |�I��I  �   end if   � ��� l  | |�H�G�H  �G  � ��� l  | |�F��F  �   "logout" of cvs.   � ��� I   | ��E�D�C�E 0 
cvs_logout  �D  �C  � ��� l  � ��B�A�B  �A  � ��@� L   � ��� b   � ���� b   � ���� b   � ���� b   � ���� b   � ���� l 	 � ���?� m   � ���  Checked in project:    �?  � o   � ��>�> 0 projecttoedit projectToEdit� m   � ��� 
 <BR>   � l 	 � ���=� m   � ���  Checkin Comment:    �=  � o   � ��<�<  0 checkincomment checkInComment� m   � ��� 
 <BR>   �@  7 ��� l     �;�:�;  �:  � ��� l      �9��9  � Q K returns a list of tokens according to AppleScript's text item delimiters.    � ��� i   9 <��� I      �8��7�8 0 tokenize  � ��6� o      �5�5 0 astring aString�6  �7  � k     4�� ��� r     ��� J     �4�4  � o      �3�3 0 itemlist itemList� ��� X    1��2�� k    ,�� ��� r    ��� l   ��1� n    ��� 1    �0
�0 
pcnt� o    �/�/ 0 	anitemref 	anItemRef�1  � o      �.�. 0 anitem anItem� ��-� Z    ,���,�+� l    ��*� >     ��� o    �)�) 0 anitem anItem� m    ��      �*  � r   # (��� b   # &��� o   # $�(�( 0 itemlist itemList� o   $ %�'�' 0 anitem anItem� o      �&�& 0 itemlist itemList�,  �+  �-  �2 0 	anitemref 	anItemRef� l   ��%� n    ��� 2  	 �$
�$ 
citm� o    	�#�# 0 astring aString�%  � ��"� L   2 4�� o   2 3�!�! 0 itemlist itemList�"  �    l     � ��   �    l      ��   2 , replaces oldChar with newChar in a string.      i   = @ I      �	�� 0 replace  	 

 o      �� 0 astring aString  o      �� 0 oldchar oldChar � o      �� 0 newchar newChar�  �   k     3  r      m            o      �� 0 	newstring 	newString  X    0� Z    +� l   � =     n      1    �
� 
pcnt  o    �� 0 achar aChar o    �� 0 oldchar oldChar�   r    #!"! b    !#$# o    �� 0 	newstring 	newString$ o     �� 0 newchar newChar" o      �� 0 	newstring 	newString�   r   & +%&% b   & )'(' o   & '�� 0 	newstring 	newString( o   ' (�� 0 achar aChar& o      �� 0 	newstring 	newString� 0 achar aChar l   
)�
) n    
*+* 2    
�	
�	 
cha + o    �� 0 astring aString�
   ,�, L   1 3-- o   1 2�� 0 	newstring 	newString�   ./. l     ���  �  / 010 i   A D232 I      �4�� 0 	substring  4 565 o      �� 0 astring aString6 7� 7 o      ���� 0 anoffset anOffset�   �  3 k     "88 9:9 r     ;<; m     ==      < o      ���� 0 
asubstring 
aSubString: >?> Y    @��AB��@ r    CDC b    EFE o    ���� 0 
asubstring 
aSubStringF l   G��G n    HIH 4    ��J
�� 
cha J o    ���� 0 	charindex 	charIndexI o    ���� 0 astring aString��  D o      ���� 0 
asubstring 
aSubString�� 0 	charindex 	charIndexA o    ���� 0 anoffset anOffsetB l   K��K I   ��L��
�� .corecnte****       ****L o    	���� 0 astring aString��  ��  ��  ? M��M L     "NN o     !���� 0 
asubstring 
aSubString��  1 OPO l     ������  ��  P QRQ i   E HSTS I      ��U���� 0 summarize_edits  U VWV o      ���� "0 aaddedfileslist aAddedFilesListW X��X o      ���� &0 aremovedfileslist aRemovedFilesList��  ��  T k     AYY Z[Z r     \]\ J     ^^ _��_ m     ``  ,    ��  ] n     aba 1    ��
�� 
txdlb 1    ��
�� 
ascr[ cdc r    efe l   g��g c    hih o    	���� "0 aaddedfileslist aAddedFilesListi m   	 
��
�� 
TEXT��  f o      ���� 0 
addedfiles 
addedFilesd jkj r    lml l   n��n c    opo o    ���� &0 aremovedfileslist aRemovedFilesListp m    ��
�� 
TEXT��  m o      ���� 0 removedfiles removedFilesk qrq r    sts J    ����  t o      ���� 0 editlist editListr uvu Z    *wx����w l   y��y >    z{z o    ���� 0 
addedfiles 
addedFiles{ m    ||      ��  x r    &}~} b    $� o     ���� 0 editlist editList� l    #���� b     #��� m     !��  added files:    � o   ! "���� 0 
addedfiles 
addedFiles��  ~ o      ���� 0 editlist editList��  ��  v ��� Z   + <������� l  + .���� >   + .��� o   + ,���� 0 removedfiles removedFiles� m   , -��      ��  � r   1 8��� b   1 6��� o   1 2���� 0 editlist editList� l  2 5���� b   2 5��� m   2 3��  removed files:    � o   3 4���� 0 removedfiles removedFiles��  � o      ���� 0 editlist editList��  ��  � ���� L   = A�� l  = @���� c   = @��� o   = >���� 0 editlist editList� m   > ?��
�� 
TEXT��  ��  R ��� l     ������  ��  � ��� i   I L��� I      ������� 0 	cvs_login  � ��� o      ���� 0 auser aUser� ���� o      ���� 0 	apassword 	aPassword��  ��  � O     ��� k    �� ��� l   �����  � 8 2 store a reference to the session, for convenience   � ��� r    
��� l   ���� 4   ���
�� 
docu� m    ���� ��  � o      ���� 0 mydoc myDoc� ��� l   ������  ��  � ��� l   �����  � + % customize the session for this user.   � ���� O    ��� k    �� ��� r    ��� o    ���� 0 auser aUser� l     ���� n      ��� 1    ��
�� 
rusr� o    ���� 0 mydoc myDoc��  � ���� r    ��� o    ���� 0 	apassword 	aPassword� l     ���� n      ��� 1    ��
�� 
pass� o    ���� 0 mydoc myDoc��  ��  � o    ���� 0 mydoc myDoc��  � m     ��null     Aߘ��  62MacCVS Pro 2.2.2 debugj?�j>j=� o��jVDj>jVD p	lib8jT� bMcvs@  alis    �   Ringo                      ��ODBD    62MacCVS Pro 2.2.2 debug                                           64��6�APPLMcvs����                  Camelot MacCVS    62  W  �  9Ringo:Camelot:tools:Camelot MacCVS:MacCVS Pro 2.2.2 debug  	 � �afpm       9 Y u � �B21 1st Flr Zone B              camelot.mcom.com               Ringo                      Patrick Beard                                     ��  � ��� l     ������  ��  � ��� i   M P��� I      �������� 0 
cvs_logout  ��  ��  � I     ������� 0 	cvs_login  � ��� m    ��      � ���� m    ��      ��  ��  � ��� l     ������  ��  � ��� i   Q T��� I      ������� 0 checkout_files  � ���� o      ���� 0 	afilelist 	aFileList��  ��  � O     ?��� k    >�� ��� l   �����  � 8 2 store a reference to the session, for convenience   � ��� r    
��� l   ���� 4   ���
�� 
docu� m    ���� ��  � o      ���� 0 mydoc myDoc� ��� l   ������  ��  � ��� l   �����  � $  checkout the specified files.   � ���� Q    >���� X    0����� k    +�� ��� r    #��� l   !���� n    !��� 1    !��
�� 
pcnt� o    ���� 0 afileref aFileRef��  � o      ���� 0 afile aFile� ���� I  $ +����
�� .MCvscoutnull        obj � o   $ %���� 0 mydoc myDoc� �����
�� 
modl� o   & '���� 0 afile aFile��  ��  �� 0 afileref aFileRef� o    ���� 0 	afilelist 	aFileList� R      ����
�� .ascrerr ****      � ****� o      ���� 0 errmsg errMsg� �� ��
�� 
errn  o      ���� 0 errnum errNum��  � k   8 >  l  8 8����   k e display dialog "The checkout could not be completed because " & errMsg & return & errNum with icon 0    �� R   8 >��
�� .ascrerr ****      � **** o   < =���� 0 errmsg errMsg ���
�� 
errn o   : ;�~�~ 0 errnum errNum�  ��  ��  � m     �� 	
	 l     �}�|�}  �|  
  i   U X I      �{�z�{ 0 checkin_files    o      �y�y 0 	afilelist 	aFileList �x o      �w�w "0 acheckincomment aCheckinComment�x  �z   O     U k    T  l   �v�v   8 2 store a reference to the session, for convenience     r    
 l   �u 4   �t
�t 
docu m    �s�s �u   o      �r�r 0 mydoc myDoc   l   �q�p�q  �p    !"! l   �o#�o  # @ : display dialog "checking into session " & (name of myDoc)   " $%$ l   �n�m�n  �m  % &'& l   �l(�l  ( #  checkin the specified files.   ' )�k) Q    T*+,* X    8-�j.- k    3// 010 r    #232 l   !4�i4 n    !565 1    !�h
�h 
pcnt6 o    �g�g 0 afileref aFileRef�i  3 o      �f�f 0 afile aFile1 7�e7 O   $ 3898 k   ( 2:: ;<; l  ( (�d=�d  = 1 + display dialog "checking in file " & aFile   < >�c> I  ( 2�b?@
�b .MCvsChKnnull        null? 4   ( ,�aA
�a 
MCflA o   * +�`�` 0 afile aFile@ �_B�^
�_ 
CmntB o   - .�]�] "0 acheckincomment aCheckinComment�^  �c  9 o   $ %�\�\ 0 mydoc myDoc�e  �j 0 afileref aFileRef. o    �[�[ 0 	afilelist 	aFileList+ R      �ZCD
�Z .ascrerr ****      � ****C o      �Y�Y 0 errmsg errMsgD �XE�W
�X 
errnE o      �V�V 0 errnum errNum�W  , k   @ TFF GHG I  @ M�UIJ
�U .sysodlogaskr        TEXTI b   @ GKLK b   @ EMNM b   @ COPO m   @ AQQ 0 *The commit could not be completed because    P o   A B�T�T 0 errmsg errMsgN o   C D�S
�S 
ret L o   E F�R�R 0 errnum errNumJ �QR�P
�Q 
dispR m   H I�O�O  �P  H S�NS R   N T�MTU
�M .ascrerr ****      � ****T o   R S�L�L 0 errmsg errMsgU �KV�J
�K 
errnV o   P Q�I�I 0 errnum errNum�J  �N  �k   m     � WXW l     �H�G�H  �G  X YZY l      �F[�F  [ O I ensures that only a valid CodeWarrior project is specified for editing.    Z \]\ i   Y \^_^ I      �E`�D�E 0 validate_project  ` a�Ca o      �B�B 0 aproject aProject�C  �D  _ k     2bb cdc r     efe l    g�Ag b     hih o     �@�@ 0 gsourcetree gSourceTreei I    �?j�>�? 0 replace  j klk o    �=�= 0 aproject aProjectl mnm m    oo  /   n p�<p m    	qq  :   �<  �>  �A  f o      �;�; 0 projectpath projectPathd rsr O    tut r    vwv e    xx n    yzy 1    �:
�: 
astyz 4    �9{
�9 
alis{ o    �8�8 0 projectpath projectPathw o      �7�7 "0 projectfiletype projectFileTypeu m    ||�null     ߀��   Finderb�Xj:�      �Ej:�j9P    o��jVDj9P b�� p	l a`      MACS   alis    ~   Ringo                      ��ODBD     Finder                                                           K�o�FNDRMACS����                  System Folder        Ringo:System Folder:Finder 	 � �afpm       9 Y u � �B21 1st Flr Zone B              camelot.mcom.com               Ringo                      Patrick Beard                                     ��  s }�6} Z     2~�5�4~ l    #��3� >    #��� o     !�2�2 "0 projectfiletype projectFileType� m   ! "�� 
 MMPr   �3   R   & .�1��0
�1 .ascrerr ****      � ****� b   ( -��� b   ( +��� m   ( )�� / )Can only edit project files. The file: ``   � o   ) *�/�/ 0 aproject aProject� m   + ,�� 0 *'' isn't a valid CodeWarrior project file.   �0  �5  �4  �6  ] ��� l     �.�-�.  �-  � ��� l      �,��,  � "  uses MacCVS to MRO a file.    � ��� i   ] `��� I      �+��*�+ 0 modify_read_only  � ��)� o      �(�( 0 	afilepath 	aFilePath�)  �*  � O     4��� k    3�� ��� l   �'��'  � 8 2 store a reference to the session, for convenience   � ��� r    
��� l   ��&� 4   �%�
�% 
docu� m    �$�$ �&  � o      �#�# 0 mydoc myDoc� ��"� O    3��� Q    2���� Z    )���!� � l   ��� >   ��� n    ��� 1    �
� 
Stat� 4    ��
� 
MCfl� o    �� 0 	afilepath 	aFilePath� m    �
� statmRod�  � I   %���
� .MCvsMRO null        null� 4    !��
� 
MCfl� o     �� 0 	afilepath 	aFilePath�  �!  �   � R      ���
� .ascrerr ****      � ****�  �  � l  1 1���  �   do nothing; it's ok   � o    �� 0 mydoc myDoc�"  � m     �� ��� l     ���  �  � ��� l      ���  � &   CW Pro IDE Interface Handlers.    � ��� l     ���  �  � ��� i   a d��� I      ���� 0 openproject openProject� ��
� o      �	�	 0 aprojectfile aProjectFile�
  �  � O     
��� k    	�� ��� l   ���  �  	 activate   � ��� I   	���
� .aevtodocnull  �    alis� o    �� 0 aprojectfile aProjectFile�  �  � m     ��null     ߀��  6�CodeWarrior IDE 3.2�Ej?�j>    o��jVDj> b�� p	l ���      CWIE   alis    �   Ringo                      ��ODBD    6�CodeWarrior IDE 3.2                                              7����APPLCWIE����                  Metrowerks CodeWarrior    6�  W  �  >Ringo:Camelot:tools:Metrowerks CodeWarrior:CodeWarrior IDE 3.2 	 � �afpm       9 Y u � �B21 1st Flr Zone B              camelot.mcom.com               Ringo                      Patrick Beard                                     ��  � ��� l     ���  �  � ��� i   e h��� I      ��� � 0 closeproject closeProject� ���� o      ���� 0 aprojectfile aProjectFile��  �   � O     
��� I   	������
�� .MMPRClsPnull    ��� null��  ��  � m     �� ��� l     ������  ��  � ��� i   i l��� I      �������� 0 
gettargets 
getTargets��  ��  � k     K�� ��� r     ��� J     ����  � o      ���� 0 
targetlist 
targetList� ��� r    	��� J    ����  � o      ���� 0 namelist nameList� ���� O   
 K��� k    J�� ��� r    ��� 4    ���
�� 
PRJD� m    ���� � o      ����  0 currentproject currentProject� ��� Y    B�������� k   % =�� ��� r   % +��� l  % )���� n   % )��� 4   & )���
�� 
TRGT� o   ' (���� 0 targetindex targetIndex� o   % &����  0 currentproject currentProject��  � o      ���� 0 currenttarget currentTarget� ��� r   , 3��� b   , 1��� o   , -���� 0 
targetlist 
targetList� J   - 0��  ��  o   - .���� 0 currenttarget currentTarget��  � o      ���� 0 
targetlist 
targetList� �� r   4 = b   4 ; o   4 5���� 0 namelist nameList J   5 : �� n   5 8	 1   6 8��
�� 
pnam	 o   5 6���� 0 currenttarget currentTarget��   o      ���� 0 namelist nameList��  �� 0 targetindex targetIndex� m    ���� � l    
��
 I    ����
�� .corecnte****       **** n     2   ��
�� 
TRGT o    ����  0 currentproject currentProject��  ��  ��  � �� L   C J K   C I ��
�� 
TRGT o   D E���� 0 
targetlist 
targetList ������ 	0 names   o   F G���� 0 namelist nameList��  ��  � m   
 ���  �  l     ������  ��    i   m p I      ������  0 gettargetfiles getTargetFiles �� o      ���� 0 	targetkey 	targetKey��  ��   k     U  r       J     ����    o      ���� 0 targetfiles targetFiles !"! O    R#$# k   	 Q%% &'& r   	 ()( 4   	 ��*
�� 
PRJD* m    ���� ) o      ����  0 currentproject currentProject' +,+ r    -.- l   /��/ n    010 4    ��2
�� 
TRGT2 o    ���� 0 	targetkey 	targetKey1 o    ����  0 currentproject currentProject��  . o      ���� 0 currenttarget currentTarget, 3��3 Y    Q4��56��4 k   ' L77 898 r   ' -:;: l  ' +<��< n   ' +=>= 4   ( +��?
�� 
SRCF? o   ) *���� 0 	fileindex 	fileIndex> o   ' (���� 0 currenttarget currentTarget��  ; o      ���� 0 
targetfile 
targetFile9 @A@ l  . .��B��  B R L only consider text files, since other platforms won't be managing binaries.   A CDC l  . .��E��  E = 7 also, only consider if target file is directly linked.   D F��F Z   . LGH����G F   . <IJI l  . 3K��K =  . 3LML n   . 1NON 1   / 1��
�� 
FTYPO o   . /���� 0 
targetfile 
targetFileM m   1 2��
�� FTYPTXTF��  J l  6 :P��P n   6 :QRQ 1   7 9��
�� 
LINKR o   6 7���� 0 
targetfile 
targetFile��  H r   ? HSTS b   ? FUVU o   ? @���� 0 targetfiles targetFilesV J   @ EWW X��X n   @ CYZY m   A C��
�� 
PATHZ o   @ A���� 0 
targetfile 
targetFile��  T o      ���� 0 targetfiles targetFiles��  ��  ��  �� 0 	fileindex 	fileIndex5 m    ���� 6 l   "[��[ I   "��\��
�� .corecnte****       ****\ n    ]^] 2   ��
�� 
SRCF^ o    ���� 0 currenttarget currentTarget��  ��  ��  ��  $ m    �" _��_ L   S U`` o   S T���� 0 targetfiles targetFiles��   aba l     ������  ��  b cdc i   q tefe I      ��g���� 0 addtargetfile addTargetFileg hih o      ���� 0 
targetfile 
targetFilei j��j o      ���� 0 
targetlist 
targetList��  ��  f O     klk I   ��mn
�� .CWIEADDFnull      P obj m l   o��o 4    ��p
�� 
PRJDp m    ���� ��  n ��qr
�� 
koclq m   	 
��
�� 
SRCFr ��st
�� 
datas J    uu v��v o    ���� 0 
targetfile 
targetFile��  t ��w��
�� 
TTGTw o    ���� 0 
targetlist 
targetList��  l m     �d xyx l     ������  ��  y z{z i   u x|}| I      ��~���� $0 setcurrenttarget setCurrentTarget~ �� o      ���� &0 currenttargetname currentTargetName��  ��  } O     
��� I   	�����
�� .MMPRSTrgnull    ��� TEXT� o    ���� &0 currenttargetname currentTargetName��  � m     �{ ��� l     ������  ��  � ��� i   y |��� I      ������� $0 removetargetfile removeTargetFile� ��� o      �~�~ 0 
targetfile 
targetFile�  ��  � O     ��� I   �}��|
�} .MMPRRemFshor  @   @ alis� J    �� ��{� o    �z�z 0 
targetfile 
targetFile�{  �|  � m     �� ��� l     �y�x�y  �x  � ��� l      �w��w  � I C adds a list of "/" delimited file paths to the specified project.    � ��� i   } ���� I      �v��u�v 0 	add_files  � ��� o      �t�t 0 aproject aProject� ��s� o      �r�r 0 	afilelist 	aFileList�s  �u  � k     ��� ��� r     ��� l    ��q� b     ��� o     �p�p 0 gsourcetree gSourceTree� I    �o��n�o 0 replace  � ��� o    �m�m 0 aproject aProject� ��� m    ��  /   � ��l� m    	��  :   �l  �n  �q  � o      �k�k 0 projectpath projectPath� ��� l   �j��j  � ? 9 display dialog "adding files to project: " & projectPath   � ��� I    �i��h�i 0 openproject openProject� ��g� o    �f�f 0 projectpath projectPath�g  �h  � ��� r    ��� I    �e�d�c�e 0 
gettargets 
getTargets�d  �c  � o      �b�b 0 targetslist targetsList� ��� r     %��� n     #��� o   ! #�a�a 	0 names  � o     !�`�` 0 targetslist targetsList� o      �_�_ 0 targetnames targetNames� ��� X   & ���^�� k   6 ��� ��� r   6 ;��� l  6 9��]� n   6 9��� 1   7 9�\
�\ 
pcnt� o   6 7�[�[ 0 afileref aFileRef�]  � o      �Z�Z 0 afile aFile� ��� r   < L��� l  < J��Y� b   < J��� o   < A�X�X 0 gsourcetree gSourceTree� I   A I�W��V�W 0 replace  � ��� o   B C�U�U 0 afile aFile� ��� m   C D��  /   � ��T� m   D E��  :   �T  �V  �Y  � o      �S�S 0 filepath filePath� ��� l  M M�R��R  � 0 * display dialog "adding file: " & filePath   � ��Q� Q   M ����� I   P W�P��O�P 0 addtargetfile addTargetFile� ��� o   Q R�N�N 0 filepath filePath� ��M� o   R S�L�L 0 targetnames targetNames�M  �O  � R      �K��
�K .ascrerr ****      � ****� o      �J�J 0 errmsg errMsg� �I��H
�I 
errn� o      �G�G 0 errnum errNum�H  � Q   _ ����� k   b w�� ��� l  b b�F��F  � F @ something failed, try closing/opening project, adding it again.   � ��� I   b h�E��D�E 0 closeproject closeProject� ��C� o   c d�B�B 0 projectpath projectPath�C  �D  � ��� I   i o�A��@�A 0 openproject openProject� ��?� o   j k�>�> 0 projectpath projectPath�?  �@  � ��=� I   p w�<��;�< 0 addtargetfile addTargetFile� ��� o   q r�:�: 0 filepath filePath� ��9� o   r s�8�8 0 targetnames targetNames�9  �;  �=  � R      �7��
�7 .ascrerr ****      � ****� o      �6�6 0 errmsg2 errMsg2� �5��4
�5 
errn� o      �3�3 0 errnum2 errNum2�4  � k    ��� � � l   �2�2   I C give up, and attempt to at least close the project before bailing.      Q    ��1 I   � ��0�/�0 0 closeproject closeProject �. o   � ��-�- 0 projectpath projectPath�.  �/   R      �,�+�*
�, .ascrerr ****      � ****�+  �*  �1   �) R   � ��(	

�( .ascrerr ****      � ****	 b   � � m   � �  Error adding files:     o   � ��'�' 0 errmsg2 errMsg2
 �&�%
�& 
errn o   � ��$�$ 0 errnum2 errNum2�%  �)  �Q  �^ 0 afileref aFileRef� o   ) *�#�# 0 	afilelist 	aFileList� �" I   � ��!� �! 0 closeproject closeProject � o   � ��� 0 projectpath projectPath�  �   �"  �  l     ���  �    l      ��   ; 5 removes a list of files from the specified project.      i   � � I      ��� 0 remove_files    o      �� 0 aproject aProject � o      �� 0 	afilelist 	aFileList�  �   k     �  !  r     "#" l    $�$ b     %&% o     �� 0 gsourcetree gSourceTree& I    �'�� 0 replace  ' ()( o    �� 0 aproject aProject) *+* m    ,,  /   + -�- m    	..  :   �  �  �  # o      �� 0 projectpath projectPath! /0/ l   �1�  1 C = display dialog "removing files from project: " & projectPath   0 232 I    �4�� 0 openproject openProject4 5�5 o    �
�
 0 projectpath projectPath�  �  3 676 Q    �89:8 k    };; <=< r    ">?> I     �	���	 0 
gettargets 
getTargets�  �  ? o      �� 0 targetslist targetsList= @A@ r   # (BCB n   # &DED o   $ &�� 	0 names  E o   # $�� 0 targetslist targetsListC o      �� 0 targetnames targetNamesA F�F X   ) }G�HG k   9 xII JKJ r   9 >LML l  9 <N� N n   9 <OPO 1   : <��
�� 
pcntP o   9 :���� 0 targetnameref targetNameRef�   M o      ���� 0 
targetname 
targetNameK QRQ I   ? E��S���� $0 setcurrenttarget setCurrentTargetS T��T o   @ A���� 0 
targetname 
targetName��  ��  R U��U X   F xV��WV k   V sXX YZY r   V [[\[ l  V Y]��] n   V Y^_^ 1   W Y��
�� 
pcnt_ o   V W���� 0 afileref aFileRef��  \ o      ���� 0 afile aFileZ `a` r   \ lbcb l  \ jd��d b   \ jefe o   \ a���� 0 gsourcetree gSourceTreef I   a i��g���� 0 replace  g hih o   b c���� 0 afile aFilei jkj m   c dll  /   k m��m m   d enn  :   ��  ��  ��  c o      ���� 0 filepath filePatha opo l  m m��q��  q R L display dialog "removing file: " & filePath & " from target: " & targetName   p r��r I   m s��s���� $0 removetargetfile removeTargetFiles t��t o   n o���� 0 filepath filePath��  ��  ��  �� 0 afileref aFileRefW o   I J���� 0 	afilelist 	aFileList��  � 0 targetnameref targetNameRefH o   , -���� 0 targetnames targetNames�  9 R      ��uv
�� .ascrerr ****      � ****u o      ���� 0 errmsg errMsgv ��w��
�� 
errnw o      ���� 0 errnum errNum��  : k   � �xx yzy Q   � �{|��{ I   � ���}���� 0 closeproject closeProject} ~��~ o   � ����� 0 projectpath projectPath��  ��  | R      ������
�� .ascrerr ****      � ****��  ��  ��  z �� R   � �����
�� .ascrerr ****      � ****� b   � ���� m   � ���  Error removing files:    � o   � ����� 0 errmsg errMsg� �����
�� 
errn� o   � ����� 0 errnum errNum��  ��  7 ���� I   � �������� 0 closeproject closeProject� ���� o   � ����� 0 projectpath projectPath��  ��  ��   ��� l     ������  ��  � ��� i   � ���� I      ������� 0 open_session  � ���� o      ���� 0 asessionpath aSessionPath��  ��  � O     ��� k    �� ��� l   �����  �   open the session file   � ���� I   �����
�� .aevtodocnull  �    alis� 4    ���
�� 
alis� o    ���� 0 asessionpath aSessionPath��  ��  � m     |� ��� l     ������  ��  � ��� l      �����  � a [ When the CGI is first loaded, this will cause MacCVS to load the Raptor CVS Session file.    � ��� l     ������  ��  � ��� i   � ���� I     ������
�� .aevtoappnull  �   � ****��  ��  � k     
�� ��� l     �����  � $  open the MacCVS session file.   � ���� I     
������� 0 open_session  � ���� o    ���� 0 gsessionpath gSessionPath��  ��  ��  � ���� l     ������  ��  ��       ��� 0������������������������������  � ������������������������������������������������������������ 0 gcamelotroot gCamelotRoot�� 0 gsourcetree gSourceTree�� 0 gsessionpath gSessionPath�� 0 crlf  �� 0 http_header  
�� .WWW�sdochtml        TEXT�� 0 query_project  �� 0 edit_project  �� 0 tokenize  �� 0 replace  �� 0 	substring  �� 0 summarize_edits  �� 0 	cvs_login  �� 0 
cvs_logout  �� 0 checkout_files  �� 0 checkin_files  �� 0 validate_project  �� 0 modify_read_only  �� 0 openproject openProject�� 0 closeproject closeProject�� 0 
gettargets 
getTargets��  0 gettargetfiles getTargetFiles�� 0 addtargetfile addTargetFile�� $0 setcurrenttarget setCurrentTarget�� $0 removetargetfile removeTargetFile�� 0 	add_files  �� 0 remove_files  �� 0 open_session  
�� .aevtoappnull  �   � ****�  Ringo:Camelot:tree:   �  Ringo:Camelot:Raptor.mcvs   �  
   � Y SHTTP/1.0 200 OK
Server: QuidProQuo
MIME-Version: 1.0
Content-type: text/html

   � �� m��������
�� .WWW�sdochtml        TEXT�� 0 	path_args  �� �����
�� 
kfor�� 0 http_search_args  � �����
�� 
post�� 0 	post_args  � �����
�� 
meth�� 0 access_method  � �����
�� 
addr�� 0 client_address  � �����
�� 
user�� 0 	user_name  � �����
�� 
pass�� 0 using_password  � ����
�� 
frmu� 0 	from_user  � �~�}�
�~ 
svnm�} 0 server_name  � �|�{�
�| 
svpt�{ 0 server_port  � �z�y�
�z 
scnm�y 0 script_name  � �x�w�
�x 
ctyp�w 0 content_type  � �v�u�
�v 
refr�u 0 referrer  � �t�s�r
�t 
Agnt�s 0 
user_agent  �r  � �q�p�o�n�m�l�k�j�i�h�g�f�e�d�c�b�a�`�_�^�]�\�[�Z�q 0 	path_args  �p 0 http_search_args  �o 0 	post_args  �n 0 access_method  �m 0 client_address  �l 0 	user_name  �k 0 using_password  �j 0 	from_user  �i 0 server_name  �h 0 server_port  �g 0 script_name  �f 0 content_type  �e 0 referrer  �d 0 
user_agent  �c 0 formdata formData�b 0 
formaction 
formAction�a 0 cvsuser cvsUser�` 0 cvspassword cvsPassword�_ 0 projecttoedit projectToEdit�^ 0 
filestoadd 
filesToAdd�] 0 filestoremove filesToRemove�\  0 filestoaddlist filesToAddList�[ &0 filestoremovelist filesToRemoveList�Z 0 error_message  � $�Y ��X�W ��V ��U � � � � � � � %'17�T�S�R�Q\�P�O�N�M�L�Kz
�Y .PCGIPCGIlist        TEXT
�X 
OFls
�W .CGIsCGI_****        TEXT
�V 
defV�U �T 0 query_project  
�S 
ascr
�R 
txdl�Q 0 tokenize  �P �O 0 edit_project  �N 0 error_message  �M  �L 0 
cvs_logout  �K  ��2�j  E�O��l E�O����� E^ O����� E^ O����� E^ O����� E^ O����� E^ O] a   )ja Y hO] a   )ja Y hO] a   )ja Y hO�a   b  a %*] ] ] m+ %Y Mb  kv_ a ,FO*] k+ E^ O*] k+ E^ Ob  a %*] ] ] ] ] a + %W 'X    
*j+ !W X "  hOb  a #%] %� �J��I�H���G�J 0 query_project  �I �F��F �  �E�D�C�E 0 cvsuser cvsUser�D 0 cvspassword cvsPassword�C  0 projecttoquery projectToQuery�H  � �B�A�@�?�>�=�<�;�:�9�8�7�6�B 0 cvsuser cvsUser�A 0 cvspassword cvsPassword�@  0 projecttoquery projectToQuery�? .0 mozillatreepathoffset mozillaTreePathOffset�> 0 query_results  �= 0 projectpath projectPath�< 0 targetslist targetsList�; 0 targetnames targetNames�: 0 targetnameref targetNameRef�9 0 
targetname 
targetName�8 0 targetfiles targetFiles�7 0 
targetfile 
targetFile�6  0 targetfilepath targetFilePath� �5�4�3�2�1�����0�/�.�-�,�+�*���)�(�'(*+�&�5 0 	cvs_login  �4 0 checkout_files  �3 0 
cvs_logout  �2 0 validate_project  
�1 .corecnte****       ****�0 0 replace  �/ 0 openproject openProject�. 0 
gettargets 
getTargets�- 	0 names  
�, 
kocl
�+ 
cobj
�* 
pcnt�) $0 setcurrenttarget setCurrentTarget�(  0 gettargetfiles getTargetFiles�' 0 	substring  �& 0 closeproject closeProject�G �*��l+  O*�kvk+ O*j+ O*�k+ Okb  j E�O�%�%E�Ob  *���m+ 	%E�O*�k+ 
O*j+ E�O��,E�O m�[��l kh ��,E�O�a %�%a %E�O*�k+ O*�k+ E�O 4�[��l kh *��,�l+ E�O�*�a a m+ 	a %%E�[OY��[OY��O*�k+ O�� �%9�$�#���"�% 0 edit_project  �$ �!��! �  � �����  0 cvsuser cvsUser� 0 cvspassword cvsPassword� 0 projecttoedit projectToEdit�  0 filestoaddlist filesToAddList� &0 filestoremovelist filesToRemoveList�#  � �������� 0 cvsuser cvsUser� 0 cvspassword cvsPassword� 0 projecttoedit projectToEdit�  0 filestoaddlist filesToAddList� &0 filestoremovelist filesToRemoveList� "0 filestocheckout filesToCheckout�  0 checkincomment checkInComment� �VW�������������
� 
bool� 0 	cvs_login  � 0 checkout_files  � 0 validate_project  � 0 modify_read_only  � 0 	add_files  � 0 remove_files  � 0 summarize_edits  � 0 checkin_files  � 0 
cvs_logout  �" ��jv 	 �jv �& �%�%Y hO*��l+ O�kv�%�%E�O*�k+ O*�k+ O*�k+ O�jv *��l+ Y hO�jv *��l+ Y hO*��l+ 	E�O*�kv�l+ 
O*j+ O�%�%�%�%�%� �
��	�����
 0 tokenize  �	 ��� �  �� 0 astring aString�  � ����� 0 astring aString� 0 itemlist itemList� 0 	anitemref 	anItemRef� 0 anitem anItem� � ���������
�  
citm
�� 
kocl
�� 
cobj
�� .corecnte****       ****
�� 
pcnt� 5jvE�O +��-[��l kh ��,E�O�� 
��%E�Y h[OY��O�� ������������ 0 replace  �� ����� �  �������� 0 astring aString�� 0 oldchar oldChar�� 0 newchar newChar��  � ������������ 0 astring aString�� 0 oldchar oldChar�� 0 newchar newChar�� 0 	newstring 	newString�� 0 achar aChar� ����������
�� 
cha 
�� 
kocl
�� 
cobj
�� .corecnte****       ****
�� 
pcnt�� 4�E�O +��-[��l kh ��,�  
��%E�Y ��%E�[OY��O�� ��3���������� 0 	substring  �� ����� �  ������ 0 astring aString�� 0 anoffset anOffset��  � ���������� 0 astring aString�� 0 anoffset anOffset�� 0 
asubstring 
aSubString�� 0 	charindex 	charIndex� =����
�� .corecnte****       ****
�� 
cha �� #�E�O ��j kh ���/%E�[OY��O�� ��T���������� 0 summarize_edits  �� ����� �  ������ "0 aaddedfileslist aAddedFilesList�� &0 aremovedfileslist aRemovedFilesList��  � ������������ "0 aaddedfileslist aAddedFilesList�� &0 aremovedfileslist aRemovedFilesList�� 0 
addedfiles 
addedFiles�� 0 removedfiles removedFiles�� 0 editlist editList� `������|���
�� 
ascr
�� 
txdl
�� 
TEXT�� B�kv��,FO��&E�O��&E�OjvE�O�� ��%%E�Y hO�� ��%%E�Y hO��&� ������������� 0 	cvs_login  �� ����� �  ������ 0 auser aUser�� 0 	apassword 	aPassword��  � �������� 0 auser aUser�� 0 	apassword 	aPassword�� 0 mydoc myDoc� �������
�� 
docu
�� 
rusr
�� 
pass�� � *�k/E�O� ���,FO���,FUU� ������������� 0 
cvs_logout  ��  ��  �  � ������ 0 	cvs_login  �� *��l+ � ������������� 0 checkout_files  �� ����� �  ���� 0 	afilelist 	aFileList��  � �������������� 0 	afilelist 	aFileList�� 0 mydoc myDoc�� 0 afileref aFileRef�� 0 afile aFile�� 0 errmsg errMsg�� 0 errnum errNum� ��������������������
�� 
docu
�� 
kocl
�� 
cobj
�� .corecnte****       ****
�� 
pcnt
�� 
modl
�� .MCvscoutnull        obj �� 0 errmsg errMsg� ������
�� 
errn�� 0 errnum errNum��  
�� 
errn�� @� <*�k/E�O ' !�[��l kh ��,E�O��l [OY��W X  	)�l�U� ������������ 0 checkin_files  �� ����� �  ������ 0 	afilelist 	aFileList�� "0 acheckincomment aCheckinComment��  � ���������������� 0 	afilelist 	aFileList�� "0 acheckincomment aCheckinComment�� 0 mydoc myDoc�� 0 afileref aFileRef�� 0 afile aFile�� 0 errmsg errMsg�� 0 errnum errNum� ��������������������Q��������
�� 
docu
�� 
kocl
�� 
cobj
�� .corecnte****       ****
�� 
pcnt
�� 
MCfl
�� 
Cmnt
�� .MCvsChKnnull        null�� 0 errmsg errMsg� ������
�� 
errn�� 0 errnum errNum��  
�� 
ret 
�� 
disp
�� .sysodlogaskr        TEXT
�� 
errn�� V� R*�k/E�O / )�[��l kh ��,E�O� *�/�l U[OY��W X 	 
�%�%�%�jl O)�l�U� ��_���������� 0 validate_project  �� ����� �  ���� 0 aproject aProject��  � ��~�}� 0 aproject aProject�~ 0 projectpath projectPath�} "0 projectfiletype projectFileType� 	oq�||�{�z����| 0 replace  
�{ 
alis
�z 
asty�� 3b  *���m+ %E�O� *�/�,EE�UO�� )j�%�%Y h� �y��x�w���v�y 0 modify_read_only  �x �u��u �  �t�t 0 	afilepath 	aFilePath�w  � �s�r�s 0 	afilepath 	aFilePath�r 0 mydoc myDoc� ��q�p�o�n�m�l�k
�q 
docu
�p 
MCfl
�o 
Stat
�n statmRod
�m .MCvsMRO null        null�l  �k  �v 5� 1*�k/E�O� % *�/�,� *�/j Y hW X  hUU� �j��i�h���g�j 0 openproject openProject�i �f��f �  �e�e 0 aprojectfile aProjectFile�h  � �d�d 0 aprojectfile aProjectFile� ��c
�c .aevtodocnull  �    alis�g � �j U� �b��a�` �_�b 0 closeproject closeProject�a �^�^   �]�] 0 aprojectfile aProjectFile�`    �\�\ 0 aprojectfile aProjectFile ��[
�[ .MMPRClsPnull    ��� null�_ � *j U� �Z��Y�X�W�Z 0 
gettargets 
getTargets�Y  �X   �V�U�T�S�R�V 0 
targetlist 
targetList�U 0 namelist nameList�T  0 currentproject currentProject�S 0 targetindex targetIndex�R 0 currenttarget currentTarget ��Q�P�O�N�M�L
�Q 
PRJD
�P 
TRGT
�O .corecnte****       ****
�N 
pnam�M 	0 names  �L �W LjvE�OjvE�O� >*�k/E�O ,k��-j kh ��/E�O��kv%E�O���,kv%E�[OY��O���U� �K�J�I�H�K  0 gettargetfiles getTargetFiles�J �G�G   �F�F 0 	targetkey 	targetKey�I   �E�D�C�B�A�@�E 0 	targetkey 	targetKey�D 0 targetfiles targetFiles�C  0 currentproject currentProject�B 0 currenttarget currentTarget�A 0 	fileindex 	fileIndex�@ 0 
targetfile 
targetFile 
��?�>�=�<�;�:�9�8�7
�? 
PRJD
�> 
TRGT
�= 
SRCF
�< .corecnte****       ****
�; 
FTYP
�: FTYPTXTF
�9 
LINK
�8 
bool
�7 
PATH�H VjvE�O� J*�k/E�O��/E�O 9k��-j kh ��/E�O��,� 	 ��,E�& ���,kv%E�Y h[OY��UO�� �6f�5�4	�3�6 0 addtargetfile addTargetFile�5 �2
�2 
  �1�0�1 0 
targetfile 
targetFile�0 0 
targetlist 
targetList�4   �/�.�/ 0 
targetfile 
targetFile�. 0 
targetlist 
targetList	 ��-�,�+�*�)�(�'
�- 
PRJD
�, 
kocl
�+ 
SRCF
�* 
data
�) 
TTGT�( 
�' .CWIEADDFnull      P obj �3 � *�k/���kv�� U� �&}�%�$�#�& $0 setcurrenttarget setCurrentTarget�% �"�"   �!�! &0 currenttargetname currentTargetName�$   � �  &0 currenttargetname currentTargetName ��
� .MMPRSTrgnull    ��� TEXT�# � �j U� ������ $0 removetargetfile removeTargetFile� ��   �� 0 
targetfile 
targetFile�   �� 0 
targetfile 
targetFile ��
� .MMPRRemFshor  @   @ alis� � 	�kvj U� ������ 0 	add_files  � ��   ��� 0 aproject aProject� 0 	afilelist 	aFileList�   ������
�	������ 0 aproject aProject� 0 	afilelist 	aFileList� 0 projectpath projectPath� 0 targetslist targetsList� 0 targetnames targetNames�
 0 afileref aFileRef�	 0 afile aFile� 0 filepath filePath� 0 errmsg errMsg� 0 errnum errNum� 0 errmsg2 errMsg2� 0 errnum2 errNum2 ������ ������������������������� 0 replace  � 0 openproject openProject� 0 
gettargets 
getTargets�  	0 names  
�� 
kocl
�� 
cobj
�� .corecnte****       ****
�� 
pcnt�� 0 addtargetfile addTargetFile�� 0 errmsg errMsg ������
�� 
errn�� 0 errnum errNum��  �� 0 closeproject closeProject�� 0 errmsg2 errMsg2 ������
�� 
errn�� 0 errnum2 errNum2��  ��  ��  
�� 
errn� �b  *���m+ %E�O*�k+ O*j+ E�O��,E�O |�[��l kh ��,E�Ob  *���m+ %E�O *��l+ W FX   *�k+ O*�k+ O*��l+ W &X   *�k+ W X  hO)a �la �%[OY��O*�k+ � ���������� 0 remove_files  �� ����   ������ 0 aproject aProject�� 0 	afilelist 	aFileList��   �������������������������� 0 aproject aProject�� 0 	afilelist 	aFileList�� 0 projectpath projectPath�� 0 targetslist targetsList�� 0 targetnames targetNames�� 0 targetnameref targetNameRef�� 0 
targetname 
targetName�� 0 afileref aFileRef�� 0 afile aFile�� 0 filepath filePath�� 0 errmsg errMsg�� 0 errnum errNum ,.������������������ln��������������� 0 replace  �� 0 openproject openProject�� 0 
gettargets 
getTargets�� 	0 names  
�� 
kocl
�� 
cobj
�� .corecnte****       ****
�� 
pcnt�� $0 setcurrenttarget setCurrentTarget�� $0 removetargetfile removeTargetFile�� 0 errmsg errMsg ������
�� 
errn�� 0 errnum errNum��  �� 0 closeproject closeProject��  ��  
�� 
errn�� �b  *���m+ %E�O*�k+ O g*j+ E�O��,E�O S�[��l kh ��,E�O*�k+ 
O 1�[��l kh ��,E�Ob  *���m+ %E�O*�k+ [OY��[OY��W &X   *�k+ W X  hO)a �la �%O*�k+ � ����������� 0 open_session  �� ����   ���� 0 asessionpath aSessionPath��   ���� 0 asessionpath aSessionPath |����
�� 
alis
�� .aevtodocnull  �    alis�� � 
*�/j U� ���������
�� .aevtoappnull  �   � ****��  ��     ���� 0 open_session  �� *b  k+  ascr  ��ޭ      (            �?< ��   �   NV��/v /0<���F&-H��/0<���F& .����g / <cpntA���"H��"�&J@fJ���gv&.��N^NuNV  /Y�/<NOTI?< ��(_ fp? ��`(/�� T Ш !@  T Ш !@ U�/ _�^>�TO(n��N^NuNV  /N��TJ g*Y�/<aplt/<scptp!�*(_ gY�//<   p �*��XON��t(n��N^Nu   
X�                     �                             ��                         ��  �          ���            ��  �        � ���          ��    �      � ���          ���     �    � �����        �����     �  ���������      �������     � ����������     ���� �     � ����� �    � �  ��    ��  ��� �   �   �  ���   ��    � ��  �     �  ����  ��      ��� �      ������� ��     ����� �      ������� ��      ������ �       ����� ��       �����         ��  ���             �               �              �  @  x   �      � " @@A  ���� @�  �  @        @  �@� �� D H X d �   @   �     �  �  �  �� �� �� �� �� ���?�������������������������?����������������������� �� �� �� �� ��  �   �    @�@ !B����A@! "D�%& @ ����?���������?���?�?�� �   �   �     ��     �   ���   �  � ����   �����  ����   ���� � ����  ����  ����  �����   ����   ��       �      aplt   FREF     �ICN#     �   APPL      x                   ����   4   $     o �_ o �(�#NuCTo run this script application, you must first install AppleScript.   @                            �Script ApplicationThis CGI script will be used to perform the edits to the given project. Using "parse CGI" OSAX to break apart the post_args.   
X� @   @       ��  ��  ��V�5�   � SIZE  �TEXT  �styl   �spsh   �pref   �WPos   �scpt   �CODE 
hfdr  "aplt  .icl4  :ICN#  Fics#  Rics4  ^BNDL  jFREF  vNOTI  �scsz  �  ��     aA�����  �r a�� ��  �� YlPh��       ���  �0    h��   �      ��   �     ���   �     ���   �     ���   �      ��  �f     ��  �    ����  �      ��  �     ���  �     ���  �     ���  ��     ���  ��     ���  �k     ���  �     ���  �      ��  �    APPLaplt! ����                  