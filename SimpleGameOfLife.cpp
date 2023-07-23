#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <random>
#include <ctime>
#include <chrono>
#include <thread>
#include <conio.h>

//возвращает набор без повторений из count значений выбранных в границах от left до right включая
bool sample(int left, int right, int count, std::vector<int>& res);

void splitRule(const std::string& rule, std::string& bornRule, std::string& surviveRule);

//при выходе val за границы диапазона [left, right) сдвигает val так как будто диапазон цикличный
int returnToRange(int val, int left, int right);

int getch_noblock() {
    if (_kbhit())
        return _getch();
    else
        return -1;
}

int main()
{
    //запрашивем у пользователя правило симуляции
    std::cout << "Enter the rule in format B3/S23\n";
    std::string rule;
    getline(std::cin, rule);
    std::regex reg("^B0?1?2?3?4?5?6?7?8?9?/S0?1?2?3?4?5?6?7?8?9?$");
    while (!std::regex_search(rule, reg))
    {
        std::cout << "The rule is not correct!\n";
        std::cout << "Enter the rule in format B3/S34\n";
        getline(std::cin, rule);
    }
    
    //разделяем правило на правила рождения и выживания
    std::string bornRule;
    std::string surviveRule;
    splitRule(rule, bornRule, surviveRule);

    //создаем пустой экран  
    const int screenHeight = 30;
    const int fieldHeight = 29;
    const int width = 120; 
    char screen[screenHeight * width + 1];
    for (int i = 0; i < screenHeight * width; ++i)
        screen[i] = ' ';
    screen[screenHeight * width] = '\0';

    
    const char filled = 219;
    const int patternSize = 800;
    std::string controlLine{ "ESC - exit    D - faster    A - slower    R - regenerate    SPACE - pause/unpause    Waiting:" };
    std::vector<int> waiting{ 1000, 800, 500, 300, 100, 50, 20 };
    while (1)
    {
        //генерируем и заполняем паттерн
        std::vector<int> selectedPositions;
        sample(0, fieldHeight * width - 1, patternSize, selectedPositions);
        for (auto iter = selectedPositions.begin(); iter != selectedPositions.end(); ++iter)
            screen[*iter] = filled;   
        
        auto curWaiting = waiting.begin();
        while (1)
        {
            //заполняем нижнюю строку, которая показывает управление
            std::string controlLineWithWaiting = controlLine + std::to_string(*curWaiting);
            //добавляем пробелов чтобы затереть предыдущую строку
            controlLineWithWaiting += std::string(width - controlLineWithWaiting.size(), ' ');
            std::copy(controlLineWithWaiting.begin(), controlLineWithWaiting.end(), std::begin(screen) + fieldHeight * width);

            std::cout << screen;

            //обрабатываем нажатия клавиш
            int received = getch_noblock();
            if (received == 27)//кнопка ESC
                return 0;
            else if (received == 100 && curWaiting + 1 != waiting.end()) //кнопка D
                ++curWaiting;
            else if (received == 97 && curWaiting != waiting.begin())//кнопка A
                --curWaiting;
            else if (received == 114)//кнопка R
                break;
            else if (received == 32)//пробел
            {
                while (getch_noblock() != 32)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(*curWaiting));

            char oldScreen[screenHeight * width + 1];
            std::copy(std::begin(screen), std::end(screen), std::begin(oldScreen));
            oldScreen[screenHeight * width] = '\0';

            for (int i = 0; i < fieldHeight * width; ++i)
            {
                int neighborsNumber = 0;
                //массив смещений для нахождения соседа
                std::vector<int> neighbors{ -1, -width - 1, -width, -width + 1, 1, width + 1, width, width - 1 };

                //проходим по всем смещениям
                for (auto iter = neighbors.begin(); iter != neighbors.end(); ++iter)
                {
                    //выполняем смещение, возвращаемся в диапазон, проверяем соседа на наличие
                    if (oldScreen[returnToRange(i + *iter, 0, fieldHeight * width)] == filled)
                        ++neighborsNumber;
                }

                if (oldScreen[i] == filled)
                {
                    //если такого числа соседей нет в правиле выживания
                    if (surviveRule.find(neighborsNumber + '0') == std::string::npos)
                        screen[i] = ' ';
                    else
                        screen[i] = filled;
                }
                else
                {
                    //если такого числа соседей нет в правиле рождения
                    if (bornRule.find(neighborsNumber + '0') == std::string::npos)
                        screen[i] = ' ';
                    else
                        screen[i] = filled;
                }
            }
        }
    }
}

bool sample(int left, int right, int count, std::vector<int>& res)
{
    if (right - left + 1 < count || left > right || count < 0)
        return false;

    std::mt19937 engine;
    engine.seed(std::time(nullptr));
    std::vector<int> seq;
    for (int i = left; i <= right; ++i)
        seq.push_back(i);

    int n = right - left + 1;
    int randVal;
    for (int i = 0; i < count; ++i)
    {
        randVal = engine() % (n - i);
        res.push_back(seq[randVal]);
        seq[randVal] = seq[n - i - 1];
    }
    return true;
}

void splitRule(const std::string& rule, std::string& bornRule, std::string& surviveRule)
{
    bornRule.clear();
    surviveRule.clear();
    auto iter = rule.begin();
    while (*iter != '/')
    {
        bornRule.push_back(*iter);
        ++iter;
    }
    ++iter;
    while (iter != rule.end())
    {
        surviveRule.push_back(*iter);
        ++iter;
    }
}

int returnToRange(int val, int left, int right)
{
    if (val < left)
        val = val % (right - left) + right - left;
    if (val >= right)
        val = val % (right - left);
    return val;
}