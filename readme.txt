5���@�i���^2���@�j�O�i��͗p�p�b�P�[�W

�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@11-Dec-2017
                              ����[��(KEK)

����́A5���@�i���^2���@�j�O�i��͗p��
�v���O�����p�b�P�[�W�ł��B
���쎲��^��Ɍ������ϑ���z�肵�Ă��܂��B

�O�i��͎菇�́A
1.Acceptance �̌v�Z
2.��͂���t�@�C�����̃��X�g����
3.��̓v���O�����̎��s
�ł��B���̃p�b�P�[�W�ɂ́A�������v�Z
����v���O�������܂܂�Ă��܂��B

��͂ł́A�Ⴆ�΁A�ˉe�ʏ�ł̋z���摜��
����ɂ́A
�u3.��̓v���O�����̎��s���ʁv
�� 
�u1.Acceptance �̌v�Z���ʁv
�Ŋ���Z���邱�Ƃœ��邱�Ƃ��ł��܂��B

-------------------
1.Acceptance �̌v�Z
-------------------
Program���F	projaccd5
�@�\�F		�ˉe�ʂ܂ł̋������L���ł���ꍇ�� acceptance ��
		�v�Z���܂��B�o�͂� csv �`���ōs���܂��B
�Ăяo���`���F	projaccd5 [options]
options:
	-dist value	�ˉe�ʂ܂ł̋����i�����j�� m �P�ʂŗ^���܂��B
			���_�́A���� XY ���j�b�g�� X �ʂ� Y�ʂ̒����ł��B
	-dzshift value	XY ���j�b�g�Ԃ́i���������́j������ mm �P�ʂŗ^���܂��B
	-out name	�o�̓t�@�C���ɂ���O�u����^���܂��B
			�o�̓t�@�C���́A���̖��O�� -0.csv�A-1.csv�A.. ��
			�t���ĕ����o�͂���܂��B

Histogram �̏o�͂�
 �@   *-0.csv, *-1.csv  �g�ݍ��킹�̐������ weight smoothing ��K�p��������
      *-2.csv, *-3.csv  acceptance ����� weight smoothing ��K�p��������
      *-4.csv, *-5.csv  acceptance �� cos^2 ���|�������̂���� weight smoothing ��
                        �K�p��������

���̃v���O�����́A�ˉe�ʂ̈ʒu�AXY ���j�b�g�ԋ������A
�􉽊w�I�Ȑݒ��ς��Ȃ���Έ�x�v�Z���邾���ŁA���̓s�x
�v�Z����K�v�͂���܂���B

--------------------------------
2.��͂���t�@�C�����̃��X�g����
--------------------------------
Program���F	gencoinlistKEK
�@�\�F		�J�n run �ԍ��A�I�� run �ԍ���^����ƁA��͂���t�@�C������
		���X�g�𐶐����܂��B��̓v���O�����́A
		���̃��X�g�ɋL�ڂ��ꂽ�t�@�C���������ɉ�͂��Ă����܂��B
		���X�g�́A�e�L�X�g�t�@�C���ŁA1�s��1�t�@�C�������L��
		����܂��B
		��̓v���O�����ł́A���̃��X�g��擪�s���珇�ɓǂ��
		��͂��A�q�X�g�O�����𐶐����܂��B

�Ăяo���`���F	gencoinlistKEK start_run_no end_run_no
	start_run_no	�J�n run number
	end_run_no	�I�� run number

��͂���f�[�^�t�@�C���� kekirid.kek.jp �� /data1/Detector5/data �ȉ���
�ۑ�����Ă�����̂Ƃ��Ă��܂��B
�W���o�͂ɏo�͂����̂ŁA�t�@�C���ɕۑ�����ɂ� re-direct ���Ă��������B


------
3.���
------
