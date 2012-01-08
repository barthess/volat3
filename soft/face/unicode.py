# -*- coding: cp1251 -*-
import pygame, sys
unistr = u"sdfdывап"
pygame.font.init()
srf = pygame.display.set_mode((640,480))
f = pygame.font.Font("cour.ttf",20)
srf.blit(f.render(unistr,True,(90,90,90)),(0,0))
pygame.display.flip()

while True:
    srf.blit(f.render(unistr,True,(255,255,255)),(0,0))
    for e in pygame.event.get():
        if e.type == pygame.QUIT:
            pygame.quit()
            sys.exit()

