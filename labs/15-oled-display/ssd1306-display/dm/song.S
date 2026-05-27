    .section .data
    .global frames_data
    .global frames_data_end
    .balign 4
frames_data:
     .incbin "mars.bin"
frames_data_end:
