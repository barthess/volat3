#!/usr/bin/python
# -*- coding: utf-8 -*-

import pygame
import datetime
import time
import ConfigParser
from gloss import *
from math import *
from multiprocessing import Process, Queue, Lock, Event, freeze_support
from Queue import Empty, Full

import globalflags
flags = globalflags.flags

# allow import from the parent directory, where mavlink.py and its stuff are
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavlink

import volatinterp

#дополнительные именованные цвета
Color.LIGHTGREY = Color(0.75, 0.75, 0.75, 1)
Color.GREY = Color(0.5, 0.5, 0.5, 1)
Color.DARKGREY = Color(0.1, 0.1, 0.1, 1)
Color.CYAN = Color(0.0, 1, 1, 1)
Color.YELLOW = Color(1, 1, 0.0, 1)

# базовый путь к файлам ресурсов
RESPATH = "resources/"

# минимальное и максимальное значение тахометра для масштабирования значения
RPM_MIN = 0.0
RPM_MAX = 3000.0

# минимальные и максимальные значения с аналоговых датчиков
AN_MIN = 0
AN_MAX = 1000

# пределы давления в кг/см^2
PRESS_MAX = 10.0
PRESS_MIN = 0.0

# read parameters
config = ConfigParser.SafeConfigParser()
config.read('default.cfg')

main_voltage_idx	= config.getint('AnalogMap', 'main_voltage_idx')
tank1_fill_idx		= config.getint('AnalogMap', 'tank1_fill_idx')
tank2_fill_idx		= config.getint('AnalogMap', 'tank2_fill_idx')
temp_oil_idx		= config.getint('AnalogMap', 'temp_oil_idx')
temp_water_idx		= config.getint('AnalogMap', 'temp_water_idx')
press_oil_idx		= config.getint('AnalogMap', 'press_oil_idx')
press_break1_idx	= config.getint('AnalogMap', 'press_break1_idx')
press_break2_idx	= config.getint('AnalogMap', 'press_break2_idx')


def discrete_dbg_print(discrete):
    st = ""
    i = 63
    while i >= 0:
        s = (discrete >> i) & 1
        st += (str(s))
        if (i % 8 == 0):
            st += " "
        i -= 1
    print st

class Label():#{{{текстовая бирка с возможностью центрирования
    def __init__(self, font):
        #Заготовка бирки с числовым значением скорости
        self.font = font
        self.halfheight = self.font.characters["0"].height / 2
        self.halfwidth  = self.font.characters["0"].width / 2

    def draw(self, text = "None", centered = True,
             position = (0,0), color = Color.WHITE):
        if centered:
            x = position[0] - self.halfwidth * len(text)
            y = position[1] - self.halfheight
            self.font.draw(text = text, position = (x, y), color = color)
        else:
            self.font.draw(text = text, position = position, color = color)
    #}}}
class Leg():#{{{аутриггеры и проколы шин
    """ Нумерация ног как в микросхеме - по кругу. """
    def __init__(self, bg, sygn_red, sygn_grey, position = (0,0)):
        self.tex_bg = Texture(RESPATH + bg)
        self.width = self.tex_bg.width
        self.height = self.tex_bg.height
        self.bg = Sprite(self.tex_bg, position = position)
        self.sygn_red = Sprite(Texture(RESPATH + sygn_red), position = position)
        self.sygn_grey = Sprite(Texture(RESPATH + sygn_grey), position = position)
        self.leg_red_lst = []
        n = 0
        while n < 6:
            n += 1
            tex = Texture(RESPATH + "leg" + str(n) + "_red.png")
            self.leg_red_lst.append(Sprite(tex, position = position))

    def draw_stat(self, flags):
        self.bg.draw()
        self.sygn_grey.draw()

    def draw_dyn(self, flags):
        if (flags & 0b111111) != 0:
            n = 0
            while n < 6:
                if ((flags >> n) & 1) == 1:
                    self.leg_red_lst[n].draw()
                n += 1
            self.sygn_red.draw()

    def draw(self, flags):
        self.draw_stat(flags)
        self.draw_dyn(flags)
    #}}}
class TiersBlock():#{{{
    def __init__(self, position = (0,0)):
        self.tier_1_left_idx  = config.getint('DiscreteMap', 'tier_1_left_idx')
        self.tier_2_left_idx  = config.getint('DiscreteMap', 'tier_2_left_idx')
        self.tier_3_left_idx  = config.getint('DiscreteMap', 'tier_3_left_idx')
        self.tier_1_right_idx = config.getint('DiscreteMap', 'tier_1_right_idx')
        self.tier_2_right_idx = config.getint('DiscreteMap', 'tier_2_right_idx')
        self.tier_3_right_idx = config.getint('DiscreteMap', 'tier_3_right_idx')
        self.leg = Leg( "tiers_bg.png",
                        "tiers_sygn_red.png",
                        "tiers_sygn_grey.png",
                        position = position)
    def draw_stat(self, globalmask):
        self.leg.draw_stat(0)
    def draw_dyn(self, globalmask):
        """ Принимает ВСЮ битовую маску из пакета телеметрии.
        На ее основе создает маску, специфичную для подкласса.
        Нумерация ног как в микросхемах - по кругу. """
        mask  = 0
        mask |= ((globalmask >> self.tier_1_left_idx)  & 1) << 0
        mask |= ((globalmask >> self.tier_2_left_idx)  & 1) << 1
        mask |= ((globalmask >> self.tier_3_left_idx)  & 1) << 2
        mask |= ((globalmask >> self.tier_3_right_idx) & 1) << 3
        mask |= ((globalmask >> self.tier_2_right_idx) & 1) << 4
        mask |= ((globalmask >> self.tier_1_right_idx) & 1) << 5
        self.leg.draw_dyn(mask)
    def draw(self, globalmask):
        draw_stat(globalmask)
        draw_dyn(globalmask)
    #}}}
class LegsBlock():#{{{
    def __init__(self, position = (0,0)):
        self.leg_1_left_idx  = config.getint('DiscreteMap', 'leg_1_left_idx')
        self.leg_2_left_idx  = config.getint('DiscreteMap', 'leg_2_left_idx')
        self.leg_1_right_idx = config.getint('DiscreteMap', 'leg_1_right_idx')
        self.leg_2_right_idx = config.getint('DiscreteMap', 'leg_2_right_idx')
        self.leg = Leg("autriggers_bg.png",
                       "autriggers_sygn_red.png",
                       "autriggers_sygn_grey.png",
                        position = position)
    def draw_stat(self, globalmask):
        self.leg.draw_stat(0)
    def draw_dyn(self, globalmask):
        """ Принимает ВСЮ битовую маску из пакета телеметрии.
        На ее основе создает маску, специфичную для подкласса.
        Нумерация ног как в микросхемах - по кругу. """
        mask  = 0
        mask |= ((globalmask >> self.leg_1_left_idx)  & 1) << 0
        mask |= ((globalmask >> self.leg_2_left_idx)  & 1) << 2
        mask |= ((globalmask >> self.leg_2_right_idx) & 1) << 3
        mask |= ((globalmask >> self.leg_1_right_idx) & 1) << 5
        self.leg.draw_dyn(mask)
    def draw(self, globalmask):
        draw_stat(globalmask)
        draw_dyn(globalmask)
    #}}}
class SymbolGrid():#{{{все-все значки дискретных датчиков
    def __init__(self):
        d01_idx = config.getint('DiscreteMap', 'd01_idx')

        self.grid_step = 85 # шаг сетки для значков
        symbol_size = 70 # значки квадратные

        self.spritelst = [] # список всех дискретных индикаторов
        self.n = 0 # вспомогательный счетчик для авторасстановки значков

        def __init_sprite(name):
            """ Функция создающая спрайты и задающая координаты согласно сетке. """
            clearance = ((self.grid_step - symbol_size) / 2) + 1
            y = 768 - 2*self.grid_step + clearance
            if self.n > 11: # значит перешли на второй ряд
                y += self.grid_step
                clearance = clearance - 12*self.grid_step

            pos = (clearance + self.grid_step * self.n, y)
            self.spritelst.append(Sprite(Texture("resources/" + name), position = pos))
            self.n += 1

        n = 0
        while n < 24:
            n += 1
            __init_sprite(str(n/10) + str(n%10) + ".png")

        # индикатор фар
        lights = Sprite(Texture(RESPATH + "lights.png"), position = (540, 330))
        self.spritelst.append(lights)
        # ручник
        parking_break = Sprite(Texture(RESPATH + "parking_break.png"), position = (640, 330))
        self.spritelst.append(parking_break)
        # движение запрещено
        stop = Sprite(Texture(RESPATH + "stop.png"), position = (510, 110))
        self.spritelst.append(stop)
        # левый поворотник
        turnleft = Sprite(Texture(RESPATH + "turn_left.png"), position = (245, 5))
        self.spritelst.append(turnleft)
        # левая часть тормзника
        clearance = 5
        tex = Texture(RESPATH + "break_left.png")
        p = (turnleft.position[0] - tex.width - clearance, turnleft.position[1])
        breakleft = Sprite(tex, position = p)
        self.spritelst.append(breakleft)
        # правый поворотник
        tex = Texture(RESPATH + "turn_right.png")
        turnright = Sprite(tex, position = (772, 5))
        self.spritelst.append(turnright)
        # правая часть тормозника
        tex = Texture(RESPATH + "break_right.png")
        p = (turnright.position[0] + turnright.texture.width + clearance, turnright.position[1])
        breakright = Sprite(tex, position = p)
        self.spritelst.append(breakright)

    def draw_stat(self, flags):
        #сетка для символики внизу экрана
        i = 1
        color = Color.DARKGREY
        s = [0, 768-self.grid_step]
        f = [1024, 768-self.grid_step]
        Gloss.draw_line(s, f, color, 2)
        s = [0, 768-2*self.grid_step]
        f = [1024, 768-2*self.grid_step]
        Gloss.draw_line(s, f, color, 2)

        while i < 12:
            s[0] = self.grid_step * i
            f[0] = s[0]
            f[1] = 768
            Gloss.draw_line(s, f, color, 2)
            i+=1

    def draw_dyn(self, flags):
        """ Принимает переменную с битовыми флагами """
        shift = 0
        for i in self.spritelst:
            if ((flags >> shift) & 1) == 1:
                i.draw()
            shift += 1

    def draw(self, flags):
        """ Принимает переменную с битовыми флагами """
        self.draw_stat(flags)
        self.draw_dyn(flags)
    #}}}
class Hand():#{{{стрелка для стрелочных приборов
    """
    Класс треугольной стрелки. Стрелка состоит из
    треугольного полигона и прямоугольника по центру. Прямоугольник
    нужен для того, чтобы кончик стрелки не был эфемерно-исчезающим.
    """
    def __init__(self, length = 100, width = 10, position = (0, 0), origin = (0, 0)):
        """Принимает:
            длину
            толщину основания
            дефолтную позицию
            координаты центра вращения
        """
        self.points = [(0.0, -width/2.0), (length, 0.0), (0.0, width/2.0)]
        self.position = position
        self.origin = origin
        self.length = length
    def draw(self, rotation = 0, color = Color.WHITE):
        """Принимает:
            угол поворота в градусах
            цвет стрелки
        """
        Gloss.draw_triangle(points = self.points,
                    position = self.position,
                    rotation = rotation,
                    origin = self.origin,
                    color = color)

        Gloss.draw_box(position = self.position,
                    width = self.length,
                    height = 1,
                    rotation = rotation,
                    origin = (self.origin[0], self.origin[1] / 2),
                    color = color)
    #}}}
class Dial():#{{{базовый метакласс для стрелочных приборов
    def __init__(self, texfile, hand, startangle = 0, endangle = 180, position = (0,0)):
        """
        Принимает:
            картинку фона
            экземпляр класса стрелки
            начальный угол в градусах
            конечный угол в градусах
            координаты
        """
        self.angle = 0.0
        self.hand = hand
        self.tex = Texture(RESPATH + texfile)
        self.dial = Sprite(self.tex, position = position)

        self.startangle = startangle
        self.endangle = endangle

        # координаты оси вращения стрелки
        x = position[0] + self.tex.width / 2
        y = position[1] + self.tex.height / 2
        self.hand.position = (x, y)

    def draw_dyn(self, val):
        val = Gloss.clamp(val, 0.0, 1.0)
        angle = self.startangle + (self.endangle - self.startangle) * val
        self.hand.draw(angle) # стрелка

    def draw_stat(self, val):
        self.dial.draw() # главный пятак

    def draw(self, val):
        """
        Принимает:
            отображаемое значение (0.0 .. 1.0), что соответствует
            минимальному и максимальному углу
        """
        self.draw_stat(val)
        self.draw_dyn(val)
    #}}}
class Speedometer():#{{{спидометр, унаследованный от базового класса
    def __init__(self, position = (0,0)):
        self.position = position
        self.hand = Hand(length = 170, width = 26, origin = (-60, 0))
        self.dial = Dial("speedometer.png", self.hand,
                         startangle = -210.0, endangle = 30.0,
                         position = position)
        self.label = Label(hugefont)

    def draw_stat(self, val):
        self.dial.draw_stat(val)

    def draw_dyn(self, val):
        self.dial.draw_dyn(val)
        speedmin = 0
        speedmax = 120
        kmph = str(int(round((speedmax - speedmin) * val)))
        x = self.position[0] + self.dial.tex.width / 2
        y = self.position[1] + self.dial.tex.height / 2
        self.label.draw(kmph, centered = True, position = (x, y))

    def draw(self, val):
        self.draw_stat(val)
        self.draw_dyn(val)
    #}}}
class Tachometer():#{{{тахометр, унаследованный от базового класса
    def __init__(self, position = (0,0)):
        self.position = position
        self.hand = Hand(length = 145, width = 22, origin = (-60, 0))
        self.dial = Dial("tachometer.png", self.hand,
                         startangle = -210.0, endangle = -90.0,
                         position = position)
        self.label = Label(normalfont)

    def draw_stat(self, val):
        self.dial.draw_stat(val)

    def draw_dyn(self, val):
        self.dial.draw_dyn(val)
        rpm = str(int(round((RPM_MAX - RPM_MIN) * val)))
        x = self.position[0] + int(self.dial.tex.width / 2.4)
        y = self.position[1] + int(self.dial.tex.height / 1.55)
        self.label.draw(rpm, centered = True, position = (x, y))

    def draw(self, val):
        self.draw_stat(val)
        self.draw_dyn(val)
    #}}}
class Counter():#{{{
    def __init__(self, capacity = 6, position = (0,0)):
        """
        Принимает:
            разрядность
        """
        self.capacity_max = 10**capacity - 1
        self.capacity_len = len(str(self.capacity_max))
        self.position = position
        self.font = smallfont
        self.halfwidth  = self.font.characters["0"].width / 2
        self.halfheight = self.font.characters["0"].height / 2
        self.bgwidth  = self.font.characters["0"].width * capacity
        self.bgheight = self.font.characters["0"].height - 1

    def draw(self, val):
        val = Gloss.clamp(val, 0, self.capacity_max)
        Gloss.draw_box(position = self.position,
                        width = self.bgwidth,
                        height = self.bgheight,
                        color = Color.WHITE)

        # определим количество ведущих нулей
        st = '0' * (self.capacity_len - len(str(int(val))))
        self.font.draw(text = st + str(val), color = Color.BLACK, position = self.position)
    #}}}
class Pressmeter():#{{{
    def __init__(self, position = (0,0)):
        self.position = position
        self.hand = Hand(length = 80, width = 16)
        self.dial = Dial("pressmeter.png", self.hand,
                         startangle = 90, endangle = -90,
                         position = position)

    def draw_stat(self, val):
        self.dial.draw_stat(val)

    def draw_dyn(self, val):
        self.dial.draw_dyn(val)

    def draw(self, val):
        self.draw_stat(val)
        self.draw_dyn(val)
    #}}}
class PressBlock():#{{{
    def __init__(self, position = (0,0)):
        step = 140
        self.pressmeter_oil = Pressmeter(position = position)
        self.pressmeter1 = Pressmeter(position = (position[0] + step, position[1]))
        self.pressmeter2 = Pressmeter(position = (position[0] + 2*step, position[1]))

        x = self.pressmeter_oil.position[0] + 110
        y = self.pressmeter_oil.position[1] + 100
        pos = [x, y]
        self.oil_sym = MulticolorSymbol("press_oil_sym_mask.png", position = pos)
        pos = [pos[0] + step, pos[1]]
        self.contour1_sym = MulticolorSymbol("press_contour1_sym_mask.png", position = pos)
        pos = [pos[0] + step, pos[1]]
        self.contour2_sym = MulticolorSymbol("press_contour2_sym_mask.png", position = pos)

        self.colornormal = Color(0, 0.5, 0, 1)
        self.coloralarm  = Color.RED

    def draw_stat(self, poil, p1, p2):
        self.oil_sym.draw(self.colornormal)
        self.contour1_sym.draw(self.colornormal)
        self.contour2_sym.draw(self.colornormal)
        self.pressmeter_oil.draw_stat(poil)
        self.pressmeter1.draw_stat(p1)
        self.pressmeter2.draw_stat(p2)

    def draw_dyn(self, poil, p1, p2):
        poil = poil / PRESS_MAX
        p1   = p1 / PRESS_MAX
        p2   = p2 / PRESS_MAX
        if poil > 0.8:
            self.oil_sym.draw(self.coloralarm)
        if p1 > 0.8:
            self.contour1_sym.draw(self.coloralarm)
        if p2 > 0.8:
            self.contour2_sym.draw(self.coloralarm)

        self.pressmeter_oil.draw_dyn(poil)
        self.pressmeter1.draw_dyn(p1)
        self.pressmeter2.draw_dyn(p2)

    def draw(self, poil, p1, p2):
        """ Принимает значения давлений масла, тормозного контура 1 и 2 в кг/см^2"""
        self.draw_stat(poil, p1, p2)
        self.draw_dyn(poil, p1, p2)
    #}}}
class Clock():#{{{ Часы с календарем, кукушкой, цыганами и медведями
    def __init__(self, position = (0,0)):
        self.position = position
    def draw(self):
        st = str(datetime.datetime.now())
        smallfont.draw(st[11:19], position = self.position)
        p = (self.position[0] + 5, self.position[1] + 35)
        footnotefont.draw(st[0:10], position = p)
    #}}}
class MulticolorSymbol():#{{{значок с возможностью задания цвета
    """ Значок рисуется с помощью цветного прямоугольника
    с последующим наложением маски нужной формы. """
    def __init__(self, maskfile, position = (0,0)):
        self.position = position
        self.tex = Texture(RESPATH + maskfile)
        self.width = self.tex.width
        self.height = self.tex.height
        self.symmask = Sprite(self.tex, position = position)
    def draw(self, color = Color.WHITE):
        # прямоугольник фона, который будет просвечивать сквозь маску значка
        Gloss.draw_box(width = self.width, height = self.height,
                       color = color, position = self.position)
        # накладываем маску
        self.symmask.draw(position = self.position)
    #}}}
class Thermometer():#{{{ термометр в виде столбика с шариком
    def __init__(self, multicolorsym, masktex, position = (0,0)):
        """ Рисует каноничный градусник в виде цветного прямоугольника
        с наложенной на него маской, изображающей прибор. Под
        градусником располагается символизирующий значок.

        Принимает:
            объект многоцветного значка
            название файла текстуры градусника
            координаты
        """
        self.tex_mask = Texture(RESPATH + masktex)
        self.mask = Sprite(self.tex_mask, position)

        self.multicolorsym = multicolorsym
        clearance = 4
        x = position[0] + (self.tex_mask.width / 2 - multicolorsym.width / 2)
        y = position[1] + self.tex_mask.height + clearance
        self.multicolorsym.position = (x, y)
    def draw(self, t):
        """ Принимает температуру, которую надо отобразить (градусы цельсия)"""
        bluet = 50
        yellowt = 70
        greent = 90
        if t < bluet:
            color = Color.CYAN
        elif t >= bluet and t < yellowt:
            color = Color.YELLOW
        elif t >= yellowt and t < greent:
            color = Color.GREEN
        else:
            color = Color.RED

        floor = 40 # degrees
        top = 120 # degrees
        ppd = 3 # pixels per degree
        minpix = 26 # столько пикселей должно быть, чтобы дотянуть до floor
        h = minpix + (Gloss.clamp(t, floor, top) - floor) * ppd
        x = self.mask.position[0] + self.tex_mask.width
        y = self.mask.position[1] + self.tex_mask.height
        Gloss.draw_box(position = (x,y),
                       width = self.tex_mask.width,
                       height = h,
                       color = color,
                       rotation = 180)

        # рисуем маску самого градусника
        self.mask.draw()
        # а теперь значок
        self.multicolorsym.draw(color)
    #}}}
class ThermoBlock():#{{{Два градусника собранные в единый блок с разметкой
    def __init__(self, position = (0,0)):
        self.position = position
        self.clearance = 60 # зазор между градусниками
        self.oilsym = MulticolorSymbol("temp_oil_sym_mask.png")
        self.thermometer_oil = Thermometer(self.oilsym, "thermometer_mask_mirror.png",
                                           position = (position[0] + self.clearance, position[1]))
        self.watersym = MulticolorSymbol("temp_water_sym_mask.png")
        self.thermometer_water = Thermometer(self.watersym, "thermometer_mask.png",
                                             position = position)
        self.label = Label(footnotefont)
    def draw(self, toil, twater):
        """ Принимает температуры масла и воды (в алфавитном порядке)"""
        self.thermometer_oil.draw(toil)
        self.thermometer_water.draw(twater)
        ppd = 3 # pixels per degree
        x = self.position[0] + 45
        y = self.position[1] + 241
        self.label.draw("40", centered = True, position = (x, y))
        self.label.draw("60", centered = True, position = (x, y - 20*ppd))
        self.label.draw("80", centered = True, position = (x, y - 40*ppd))
        self.label.draw("100", centered = True, position = (x, y - 60*ppd))
    #}}}
class Tank():#{{{Индикатор соляры
    def __init__(self, position = (0,0)):
        self.position = position
        self.fuelsym = MulticolorSymbol("fuel_sym_mask.png", position = position)

        # координаты линий прямоугольной рамки, символизирующей бак
        self.width = 25
        self.height = 260
        p1 = (self.position[0] + self.width, self.position[1])
        p2 = (p1[0], p1[1] + self.height)
        p3 = (p2[0] - self.width, p2[1])
        self.lines = [self.position, p1, p2, p3]
        # координаты линий разметки на баках
        start = (self.position[0] + self.width, self.position[1] + int(self.height / 2))
        finish = (start[0] - int(self.width / 1.5), start[1])
        p1 = (start, finish)
        start = start[0], self.position[1] + int(self.height * 0.25) + 0.5
        finish = (start[0] - int(self.width / 2), start[1])
        p2 = (start, finish)
        start = start[0], self.position[1] + int(self.height * 0.75) + 0.5
        finish = (start[0] - int(self.width / 2), start[1])
        p3 = (start, finish)
        self.gridlines = [p1, p2, p3]
        # прикинем координаты для значка
        x = self.position[0] - 5
        y = self.position[1] + self.height + 5
        self.fuelsym.position = (x, y)
        # именованные цвета
        self.normalcolor_sym  = Color.GREEN
        self.alarmcolor_sym   = Color.RED
        self.normalcolor_tank = Color(0, 1, 0, 0.5)
        self.alarmcolor_tank  = Color(1, 0, 0, 0.5)

    def draw_stat(self, val):
        self.fuelsym.draw(self.normalcolor_sym)

    def draw_dyn(self, val):
        val = Gloss.clamp(val, 0, 1)
        if val < 0.25:
            color = self.alarmcolor_tank
            self.fuelsym.draw(self.alarmcolor_sym)
        else:
            color = self.normalcolor_tank
        # tank
        position = (self.position[0] + self.width, self.position[1] + self.height)
        Gloss.draw_box(position,
                width = self.width,
                height = self.height * val,
                color = color,
                rotation = 180)
        # рамка
        Gloss.draw_lines(self.lines, Color.WHITE, width = 2, join = True)
        # разметка
        Gloss.draw_line(self.gridlines[0][0], self.gridlines[0][1], color = Color.WHITE, width = 2)
        Gloss.draw_line(self.gridlines[1][0], self.gridlines[1][1], color = Color.WHITE, width = 1)
        Gloss.draw_line(self.gridlines[2][0], self.gridlines[2][1], color = Color.WHITE, width = 1)

    def draw(self, val):
        self.draw_stat(val)
        self.draw_dyn(val)
    #}}}
class FuelBlock():#{{{Два индикатора в моноблоке
    def __init__(self, position = (0,0)):
        self.position = position
        self.clearance = 50
        self.tank1 = Tank(position = position)
        self.tank2 = Tank(position = (position[0] + self.clearance, position[1]))

    def draw_stat(self, val1, val2):
        self.tank1.draw_stat(val1)
        self.tank2.draw_stat(val2)

    def draw_dyn(self, val1, val2):
        self.tank1.draw(val1)
        self.tank2.draw(val2)
        x = self.position[0] + self.tank1.width / 2 - 9
        y = self.position[1] + self.tank1.height + 20
        normalfont.draw(text = "1", position = (x, y), color = Color.BLACK)
        normalfont.draw(text = "2", position = (x + self.clearance, y), color = Color.BLACK)

    def draw(self, val1, val2):
        self.draw_stat(val1, val2)
        self.draw_dyn(val1, val2)
    #}}}
class Battery():#{{{батарейка с цикверками
    def __init__(self, position = (0,0)):
        self.position = position
        self.sym = MulticolorSymbol("battery_sym_mask.png", position = position)

    def draw_stat(self, val):
        color = Color.GREEN
        self.sym.draw(color = color)

    def draw_dyn(self, val):
        if val < 21:
            color = Color.RED
            self.sym.draw(color = color)
        string = str(round(val, 1)) + " B"
        stringpos = self.position[0] + 9, self.position[1] + 32
        footnotefont.draw(string, position = stringpos)

    def draw(self, val):
        self.draw_stat(val)
        self.draw_dyn(val)
    #}}}
class ATM():#{{{ Меню а ля банкомат
    def __init__(self):
        pass
    def draw(self):
        Gloss.draw_box(width = 1024, height = 768, color = Color(0,0,0,0.85))
        buttonw = 200
        buttonh = 64
        i = 0
        while i < 6:
            Gloss.draw_box(position = (0, buttonh + 128*i), width = buttonw, height = buttonh, color = Color.GREEN)
            i += 1
#}}}
class WarningWindow():#{{{
    def __init__(self):
        pass
    def draw(self, string):
        Gloss.draw_box(width = 1024, height = 128, color = Color.RED)
        largefont.draw(string)
        pass
    #}}}


class Telemetry(GlossGame):#{{{
    def init(self, q_tlm):#{{{
        #проброс глобальных переменных внутрь класса.
        self.q_tlm = q_tlm
        #
        #}}}
    def preload_content(self):#{{{
        self.t = Texture("resources/loadscreen.png")
        pass
    #}}}
    def draw_loading_screen(self):#{{{
        Sprite(self.t).draw()
        pass
    #}}}
    def load_content(self):#{{{

        self.mousepos = (0, 0)
        self.on_mouse_motion = self.handle_mouse_motion
        # для обработки кликов
        self.on_mouse_down = self.handle_mouse_clicks
        # для обработки клавиш
        self.on_key_up = self.handle_key_presses
        # переменная для слежения за отсутствием присутствия увву
        self.last_success_time = time.time()

        # всякие полезные шрифты
        global tinyfont
        global footnotefont
        global smallfont
        global normalfont
        global hugefont
        global largefont
        tinyfont     = SpriteFont(RESPATH + "DroidSansMono.ttf", size = 12, startcharacter = 32, endcharacter = 126)
        footnotefont = SpriteFont(RESPATH + "DroidSansMono.ttf", size = 18, startcharacter = 32, endcharacter = 126)
        smallfont    = SpriteFont(RESPATH + "DroidSansMono.ttf", size = 25, startcharacter = 32, endcharacter = 126)
        normalfont   = SpriteFont(RESPATH + "DroidSansMono.ttf", size = 32, startcharacter = 32, endcharacter = 126)
        largefont    = SpriteFont(RESPATH + "DroidSansMono.ttf", size = 48, startcharacter = 32, endcharacter = 126)
        hugefont     = SpriteFont(RESPATH + "DroidSansMono.ttf", size = 64, startcharacter = 32, endcharacter = 126)

        # константы для расчета координат объектов
        legh = 220
        legw = 150
        symgridh = 85
        symgridw = 85
        pressh = 180
        pressw = 180

        # объявим отображаемые значения. Обновляются функцией update
        self.speed = 0.0
        self.tacho = 0.0
        self.engine_uptime = 0
        self.trip_value = 0
        self.discrete_msk = 0
        self.temp_oil = 0.0
        self.temp_water = 0.0
        self.main_voltage = 0.0
        self.tank1_fill = 0.0
        self.tank2_fill = 0.0
        self.press_oil = 0.0
        self.press_break1 = 0.0
        self.press_break2 = 0.0

        # инициализация графических объектов
        self.speedometer = Speedometer(position = (380,10))
        self.tachometer = Tachometer(position = (145,45))
        self.thermoblock = ThermoBlock(position = (920,5))
        self.symgrid = SymbolGrid()
        self.autriggers = LegsBlock(position = (5, 768 - legh - 2*symgridh - 5))
        self.tiers = TiersBlock(position = (1024 - legw - 5, 768 - legh - 2*symgridh - 5))
        self.trip = Counter(capacity = 6, position = (750, 250))
        self.motohours = Counter(capacity = 4, position = (315, 120))
        self.fuelblock = FuelBlock(position = (10, 10))
        self.pressblock = PressBlock(position = (235, 768 - pressh - 2*symgridh - 5))
        self.battery = Battery(position = (180, 470))
        self.clock = Clock(position = (730, 480))
        self.atm = ATM()
        self.warning = WarningWindow()

        # инициализация датчика ТМ100 (темометр)
        tm100_x = [57,  104, 115, 133, 151, 176, 201, 235, 266, 308, 354, 406, 556] # Ohm
        tm100_y = [120, 100,  95,  90,  85,  80,  75,  70,  65,  60,  55,  50,  40] # Celsius
        self.TM100 = volatinterp.interp1d(tm100_x, tm100_y, AN_MIN, AN_MAX)

        # инициализация датчика 18.3829 (давленометр)
        mzkt18_x = [28, 38, 50, 63, 75, 87, 104, 119, 137, 155, 168] # Ohm
        mzkt18_y = [10,  9,  8,  7,  6,  5,   4,   3,   2,   1,   0] # kg/cm^2
        self.mzkt18 = volatinterp.interp1d(mzkt18_x, mzkt18_y, AN_MIN, AN_MAX)

        # датчик безнина (ДУМП-02)
        self.dump02 = volatinterp.linear(0, 0, 100, 1)

        # self.particles = ParticleSystem(Texture(RESPATH + "01.png"),position = (320, 320), initialparticles = 0, lifespan = 30000, creationspeed = 10, growth = 4.0, wind = (50,0), minspeed = 5, maxspeed = 50)

        # сгенерим статичный задник
        Gloss.clear(Color.BLACK)
        self.speedometer.draw_stat(0)
        self.tachometer.draw_stat(0)
        self.pressblock.draw_stat(0, 0, 0)
        self.battery.draw_stat(24)
        self.fuelblock.draw_stat(0.5, 0.5)
        self.autriggers.draw_stat(0)
        self.tiers.draw_stat(0)
        self.symgrid.draw_stat(0)
        Gloss.save_screenshot("/tmp/static_bg.png")
        self.bgtexture = Texture("/tmp/static_bg.png")
    #}}}
    def draw(self):#{{{
        """The draw() method of your game automatically gets called by Gloss
        every frame, allowing you to render your game to the screen. If you
        want to update things in your game, eg moving this new sprite, you
        shouldn't put that code here. Instead, create an update() method
        inside your game's class
        """
        Gloss.fill(self.bgtexture)
        # self.particles.draw()
        self.motohours.draw(self.engine_uptime)
        self.tachometer.draw_dyn(self.tacho)
        self.trip.draw(self.trip_value)
        self.speedometer.draw_dyn(self.speed)
        self.thermoblock.draw(self.temp_oil, self.temp_water)
        self.pressblock.draw_dyn(self.press_oil, self.press_break1, self.press_break2)
        self.fuelblock.draw_dyn(self.tank1_fill, self.tank2_fill)
        self.battery.draw_dyn(self.main_voltage)

        self.autriggers.draw_dyn(self.discrete_msk)
        self.tiers.draw_dyn(self.discrete_msk)
        self.symgrid.draw_dyn(self.discrete_msk)

        self.clock.draw()

        if flags["atm_mode"] is True:
            self.atm.draw()

        if (time.time() - self.last_success_time) > 2:
            self.warning.draw("UVVU not responding\n")
        #self.tv.draw()
        #}}}
    def update(self):#{{{
        """ Обновляет всякие флаги, согласно которым рисуется всякая ботва. """
        if flags["debug"] is True:
            self.__debugupdate()
        else:
            self.__normalupdate()
        #time.sleep(0.1)
        #}}}
    def handle_key_presses(self, event):#{{{
        if event.key is K_SPACE:
            self.flags["atm_mode"] = not self.flags["atm_mode"]
    #}}}
    def handle_mouse_clicks(self, event):#{{{
        pygame.display.set_caption(str(event.pos[0]) + "x" + str(event.pos[1]))
        #}}}
    def handle_mouse_motion(self, event):#{{{
        if flags["mouse_capture"] is True:
            self.mousepos = event.pos
        #}}}
    def __normalupdate(self):#{{{
        """ Стандартная обновлялка инфы """
        tlm_data = None

        try:
            tlm_data = self.q_tlm.get(True, 1)
        except Empty:
            pass
        else:
            # are we needed this data?
            if type(tlm_data) == mavlink.MAVLink_mpiovd_sensors_raw_message:
                self.last_success_time = time.time()
                analogarray = [tlm_data.analog00, tlm_data.analog01, tlm_data.analog02, tlm_data.analog03,
                               tlm_data.analog04, tlm_data.analog05, tlm_data.analog06, tlm_data.analog07,
                               tlm_data.analog08, tlm_data.analog09, tlm_data.analog10, tlm_data.analog11,
                               tlm_data.analog12, tlm_data.analog13, tlm_data.analog14, tlm_data.analog15]
            else:
                tlm_data = None

        if tlm_data is not None:
            # растусовка всей ботвы из пакета
            self.speed = tlm_data.analog00 / 100000.0 #tlm_data.speed / 256.0
            self.tacho = tlm_data.rpm / RPM_MAX
            self.trip_value = tlm_data.trip
            self.engine_uptime = tlm_data.engine_uptime
            # analog sensors
            self.main_voltage   = analogarray[main_voltage_idx] / 1000.0
            self.tank1_fill     = self.dump02.get(analogarray[tank1_fill_idx])
            self.tank2_fill     = self.dump02.get(analogarray[tank2_fill_idx])
            self.temp_oil       = self.TM100.get(Gloss.clamp(analogarray[temp_oil_idx], AN_MIN, AN_MAX))
            self.temp_water     = self.TM100.get(Gloss.clamp(analogarray[temp_water_idx], AN_MIN, AN_MAX))
            self.press_oil      = self.mzkt18.get(Gloss.clamp(analogarray[press_oil_idx], AN_MIN, AN_MAX))
            self.press_break1   = self.mzkt18.get(Gloss.clamp(analogarray[press_break1_idx], AN_MIN, AN_MAX))
            self.press_break2   = self.mzkt18.get(Gloss.clamp(analogarray[press_break2_idx], AN_MIN, AN_MAX))
            # discrete sensors
            self.discrete_msk   = tlm_data.relay
            discrete_dbg_print(self.discrete_msk)
            # и в самом конце "сбрасываем флаг"
            tlm_data = None
        #}}}
    def __debugupdate(self):#{{{
        """ Отладочная обновлялка информации на экране """
        self.last_success_time = time.time()

        self.speed = self.mousepos[1] / 768.0
        self.tacho = self.speed
        self.temp_oil = self.speed * 120
        self.temp_water = self.speed * 300

        #self.autriggers_msk += 1
        #self.autriggers_msk &= 2**7 - 1
        self.sym_msk = 1 << (self.mousepos[0] / 32)
        #}}}
#}}}

