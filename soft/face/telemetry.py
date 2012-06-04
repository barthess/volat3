#!/usr/bin/python
# -*- coding: utf-8 -*-

import pygame
import datetime
import time
from gloss import *
from math import *
from multiprocessing import Process, Queue, Lock, Event, freeze_support
from Queue import Empty, Full

from dsp import *

import globalflags
flags = globalflags.flags

# allow import from the parent directory, where mavlink.py and its stuff are
sys.path.insert(0, os.path.join(os.path.dirname(os.path.realpath(__file__)), '../mavlink/python'))
import mavlink

#дополнительные именованные цвета
Color.LIGHTGREY = Color(0.75, 0.75, 0.75, 1)
Color.GREY = Color(0.5, 0.5, 0.5, 1)
Color.DARKGREY = Color(0.1, 0.1, 0.1, 1)
Color.CYAN = Color(0.0, 1, 1, 1)
Color.YELLOW = Color(1, 1, 0.0, 1)

# базовый путь к файлам ресурсов
RESPATH = "resources/"

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

    def draw(self, flags):
        self.bg.draw()
        n = 0
        failed = False
        while n < 6:
            if ((flags >> n) & 1) == 1:
                failed = True
                self.leg_red_lst[n].draw()
            n += 1
        if failed:
            self.sygn_red.draw()
        else:
            self.sygn_grey.draw()
    #}}}
class SymbolGrid():#{{{все-все значки дискретных датчиков
    def __init__(self):
        self.grid_step = 85 # шаг сетки для значков
        symbol_size = 70 # значки квадратные

        self.spritelst = [] # список всех дискретных индикаторов
        self.n = 0 # вспомогательный счетчик для авторасстановки значков

        def init_sprite(name):
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
            init_sprite(str(n/10) + str(n%10) + ".png")

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

    def draw(self, flags):
        """ Принимает переменную с битовыми флагами """
        shift = 0
        for i in self.spritelst:
            if ((flags >> shift) & 1) == 1:
                i.draw()
            shift += 1

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

    def draw(self, val):
        """
        Принимает:
            отображаемое значение (0.0 .. 1.0), что соответствует
            минимальному и максимальному углу
        """
        val = Gloss.clamp(val, 0.0, 1.0)
        self.dial.draw() # главный пятак
        angle = self.startangle + (self.endangle - self.startangle) * val
        self.hand.draw(angle) # стрелка
    #}}}
class Speedometer():#{{{спидометр, унаследованный от базового класса
    def __init__(self, position = (0,0)):
        self.position = position
        self.hand = Hand(length = 170, width = 26, origin = (-60, 0))
        self.dial = Dial("speedometer.png", self.hand,
                         startangle = -210.0, endangle = 30.0,
                         position = position)
        self.label = Label(hugefont)

    def draw(self, val):
        self.dial.draw(val)
        speedmin = 0
        speedmax = 120
        kmph = str(int(round((speedmax - speedmin) * val)))
        x = self.position[0] + self.dial.tex.width / 2
        y = self.position[1] + self.dial.tex.height / 2
        self.label.draw(kmph, centered = True, position = (x, y))
    #}}}
class Tachometer():#{{{тахометр, унаследованный от базового класса
    def __init__(self, position = (0,0)):
        self.position = position
        self.hand = Hand(length = 145, width = 22, origin = (-60, 0))
        self.dial = Dial("tachometer.png", self.hand,
                         startangle = -210.0, endangle = -90.0,
                         position = position)
        self.label = Label(normalfont)

    def draw(self, val):
        self.dial.draw(val)
        rpmmin = 0
        rpmmax = 3000
        rpm = str(int(round((rpmmax - rpmmin) * val)))
        x = self.position[0] + int(self.dial.tex.width / 2.4)
        y = self.position[1] + int(self.dial.tex.height / 1.55)
        self.label.draw(rpm, centered = True, position = (x, y))
    #}}}
class Counter():#{{{
    def __init__(self, capacity = 6, position = (0,0)):
        """
        Принимает:
            разрядность
        """
        self.capacity_len = len(str(10**capacity - 1))
        self.position = position
        self.font = smallfont
        self.halfwidth  = self.font.characters["0"].width / 2
        self.halfheight = self.font.characters["0"].height / 2
        self.bgwidth  = self.font.characters["0"].width * capacity
        self.bgheight = self.font.characters["0"].height - 1

    def draw(self, val):
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
    def draw(self, val):
        self.dial.draw(val)
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
    def draw(self, poil, p1, p2):
        """ Принимает значения давлений масла, тормозного контура 1 и 2"""
        if poil > 0.8:
            self.oil_sym.draw(self.coloralarm)
        else:
            self.oil_sym.draw(self.colornormal)
        if p1 > 0.7:
            self.contour1_sym.draw(self.coloralarm)
        else:
            self.contour1_sym.draw(self.colornormal)
        if p2 > 0.9:
            self.contour2_sym.draw(self.coloralarm)
        else:
            self.contour2_sym.draw(self.colornormal)

        self.pressmeter_oil.draw(poil)
        self.pressmeter1.draw(p1)
        self.pressmeter2.draw(p2)
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
        pass
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
        """ Принимает температуру, которую надо отобразить """
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

        #нарисуем рамку, символизирующую бак
        self.width = 25
        self.height = 260
        p1 = (self.position[0] + self.width, self.position[1])
        p2 = (p1[0], p1[1] + self.height)
        p3 = (p2[0] - self.width, p2[1])
        self.lines = [self.position, p1, p2, p3]

        # прикинем координаты для значка
        x = self.position[0] - 5
        y = self.position[1] + self.height + 5
        self.fuelsym.position = (x, y)

    def draw(self, val):
        redval = 0.25
        if val < redval:
            color = Color.RED
        else:
            color = Color.GREEN

        self.fuelsym.draw(color)
        position = (self.position[0] + self.width, self.position[1] + self.height)
        Gloss.draw_box(position,
                width = self.width,
                height = self.height * val,
                color = color,
                rotation = 180)

        # прямоугольник для легкого затемнения цвета
        Gloss.draw_box(position = self.position,
                width = self.width,
                height = self.height,
                color = Color(0,0,0,0.5))
        # рамка
        Gloss.draw_lines(self.lines, Color.WHITE, width = 2, join = True)

        # разметка
        start = (self.position[0] + self.width, self.position[1] + int(self.height / 2))
        finish = (start[0] - int(self.width / 1.5), start[1])
        Gloss.draw_line(start, finish, color = Color.WHITE, width = 2)
        start = start[0], self.position[1] + int(self.height * 0.25) + 0.5
        finish = (start[0] - int(self.width / 2), start[1])
        Gloss.draw_line(start, finish, color = Color.WHITE, width = 1)
        start = start[0], self.position[1] + int(self.height * 0.75) + 0.5
        finish = (start[0] - int(self.width / 2), start[1])
        Gloss.draw_line(start, finish, color = Color.WHITE, width = 1)
    #}}}
class FuelBlock():#{{{Два индикатора в моноблоке
    def __init__(self, position = (0,0)):
        self.position = position
        self.clearance = 50
        self.tank1 = Tank(position = position)
        self.tank2 = Tank(position = (position[0] + self.clearance, position[1]))
    def draw(self, val1, val2):
        self.tank1.draw(Gloss.clamp(val1, 0, 1))
        self.tank2.draw(Gloss.clamp(val2, 0, 1))
        x = self.position[0] + self.tank1.width / 2 - 9
        y = self.position[1] + self.tank1.height + 20
        normalfont.draw(text = "1", position = (x, y), color = Color.BLACK)
        normalfont.draw(text = "2", position = (x + self.clearance, y), color = Color.BLACK)
    #}}}
class Battery():#{{{батарейка с цикверками
    def __init__(self, position = (0,0)):
        self.position = position
        self.sym = MulticolorSymbol("battery_sym_mask.png", position = position)

    def draw(self, val):
        if val < 21:
            color = Color.RED
        else:
            color = Color.GREEN
        self.sym.draw(color = color)
        string = str(round(val, 1)) + " B"
        stringpos = self.position[0] + 9, self.position[1] + 32
        footnotefont.draw(string, position = stringpos)
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
        # переменная для слежения за присутствием увву
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

        # отображаемые значения. Обновляются функцией update
        self.speed = 0.0
        self.tacho = 0.0
        self.temp_oil = 0.0
        self.temp_water = 0.0

        # инициализация объектов
        self.speedometer = Speedometer(position = (380,10))
        self.tachometer = Tachometer(position = (145,45))
        self.thermoblock = ThermoBlock(position = (920,5))

        self.sym_msk = 2**32 - 1
        self.symgrid = SymbolGrid()

        self.autriggers_msk = 0
        self.autriggers = Leg("autriggers_bg.png",
                              "autriggers_sygn_red.png",
                              "autriggers_sygn_grey.png",
                              position = (5, 768 - legh - 2*symgridh - 5))

        self.tiers_msk = 0
        self.tiers = Leg("tiers_bg.png",
                         "tiers_sygn_red.png",
                         "tiers_sygn_grey.png",
                         position = (1024 - legw - 5, 768 - legh - 2*symgridh - 5))

        self.trip = Counter(capacity = 6, position = (750, 250))
        self.motohours = Counter(capacity = 4, position = (315, 120))
        self.fuelblock = FuelBlock(position = (10, 10))

        self.pressblock = PressBlock(position = (235, 768 - pressh - 2*symgridh - 5))
        self.battery = Battery(position = (180, 470))
        self.clock = Clock(position = (730, 480))

        self.atm = ATM()
        self.warning = WarningWindow()

        #self.tv = Sprite(Texture(RESPATH + "tv.png"), position = (400, 400))
        #time.sleep(1)# имитация непосильной работы по загрузке ресурсов
    #}}}
    def draw(self):#{{{
        """The draw() method of your game automatically gets called by Gloss
        every frame, allowing you to render your game to the screen. If you
        want to update things in your game, eg moving this new sprite, you
        shouldn't put that code here. Instead, create an update() method
        inside your game's class
        """

        Gloss.clear(Color.BLACK)
        self.symgrid.draw(self.sym_msk)
        self.motohours.draw(7)
        self.tachometer.draw(self.tacho)
        self.trip.draw(31)
        self.speedometer.draw(self.speed)
        self.thermoblock.draw(self.temp_oil, self.temp_water / 1.5)
        self.autriggers.draw(self.autriggers_msk)
        self.tiers.draw(self.tiers_msk)
        self.pressblock.draw(self.speed * 1.2, self.speed * 0.3, self.speed + 0.5)
        self.fuelblock.draw(self.speed + 0.3, self.speed)
        self.battery.draw(self.speed * 40)
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
            tlm_data = self.q_tlm.get_nowait()
        except Empty:
            pass
        else:
            # are we needed this data?
            if type(tlm_data) == mavlink.MAVLink_mpiovd_sensors_raw_message:
                self.last_success_time = time.time()
            else:
                tlm_data = None

        if tlm_data is not None:
            # растусовка всей ботвы из пакета
            self.speed = tlm_data.speed / 256.0
            self.tacho = tlm_data.rpm / 256.0
            self.temp_oil = tlm_data.analog01
            self.temp_water = tlm_data.analog02

            self.sym_msk = tlm_data.relay
            # и в самом конце "сбрасываем флаг"
            tlm_data = None

        # моргание значками в случайном порядке
        # self.autriggers_msk += 1
        # self.autriggers_msk &= 2**7 - 1
        # self.sym_msk = random.randint(0, (2**32 - 1))
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

