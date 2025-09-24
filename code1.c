sharp
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Windows.Forms;
using System.Xml;
using System.IO;
using System.Linq;

namespace NatureQuiz
{
    public partial class MainForm : Form
    {
        private string xmlFilePath = "questions.xml";  // Путь к XML-файлу
        private XmlDocument xmlDoc;
        private Random random = new Random();

        private string currentTheme; // Выбранная тема
        private int currentLevel = 1; // Текущий уровень
        private List<Question> questions = new List<Question>(); // Список вопросов текущего уровня
        private int currentQuestionIndex = 0; // Индекс текущего вопроса
        private int score = 0; // Текущий счет

        private int timeRemaining = 60; // Время на вопрос/уровень
        private Timer timer;

        public MainForm()
        {
            InitializeComponent(); // Обязательно
            Load += MainForm_Load; // Подписываемся на событие Load

            // Инициализация таймера
            timer = new Timer();
            timer.Interval = 1000; // 1 секунда
            timer.Tick += Timer_Tick;
        }

        private void MainForm_Load(object sender, EventArgs e)
        {
            // Загрузка XML при запуске
            LoadXmlData();
            ShowMainMenu();
        }

        // Класс для представления вопроса
        public class Question
        {
            public string ImagePath { get; set; }
            public string Hint { get; set; }
            public string Text { get; set; }
            public List<string> Answers { get; set; }
            public int CorrectAnswerIndex { get; set; }
            public int Id { get; set; } // Добавили Id

            public Question()
            {
                Answers = new List<string>();
            }
        }

        // Методы для работы с XML (чтение, запись)
        private void LoadXmlData()
        {
            xmlDoc = new XmlDocument();
            try
            {
                if (!File.Exists(xmlFilePath))
                {
                    MessageBox.Show("XML file not found.  The application will not work properly.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }

                xmlDoc.Load(xmlFilePath);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error loading XML data: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        // Пример сохранения XML (изменен, чтобы создавать папки)
        private void SaveXmlData()
        {
            try
            {
                string directoryPath = Path.GetDirectoryName(xmlFilePath);
                if (!Directory.Exists(directoryPath))
                {
                    Directory.CreateDirectory(directoryPath);
                }
                xmlDoc.Save(xmlFilePath);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error saving XML data: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }


        // Методы для отображения окон
        private void ShowMainMenu()
        {
            // Пример: Создайте кнопки "Начать игру", "Администрирование", "Выход"
            // Обработчики для этих кнопок
            // Скройте/удалите другие элементы управления
            Controls.Clear();

            Button startButton = new Button();
            startButton.Text = "Начать игру";
            startButton.Click += StartButton_Click;
            startButton.Location = new Point(10, 10);
            Controls.Add(startButton);

            Button adminButton = new Button();
            adminButton.Text = "Администрирование";
            adminButton.Click += AdminButton_Click;
            adminButton.Location = new Point(10, 50);
            Controls.Add(adminButton);

            Button exitButton = new Button();
            exitButton.Text = "Выход";
            exitButton.Click += ExitButton_Click;
            exitButton.Location = new Point(10, 90);
            Controls.Add(exitButton);
        }

        private void StartButton_Click(object sender, EventArgs e)
        {
            ShowThemeSelection();
        }

        private void AdminButton_Click(object sender, EventArgs e)
        {
            ShowAdminPanel();
        }

        private void ExitButton_Click(object sender, EventArgs e)
        {
            Application.Exit();
        }

        private void ShowThemeSelection()
        {
            Controls.Clear();

            // Создание списка тем из XML
            List<string> themes = GetThemesFromXml();
            if (themes.Count == 0)
            {
                MessageBox.Show("No themes found. Please add themes using the admin panel.", "No themes", MessageBoxButtons.OK, MessageBoxIcon.Information);
                ShowMainMenu();
                return;
            }

            ComboBox themeComboBox = new ComboBox();
            themeComboBox.Items.AddRange(themes.ToArray());
            themeComboBox.Location = new Point(10, 10);
            Controls.Add(themeComboBox);

            Button selectButton = new Button();
            selectButton.Text = "Выбрать";
            selectButton.Click += (s, e) =>
            {
                if (themeComboBox.SelectedItem != null)
                {
                    currentTheme = themeComboBox.SelectedItem.ToString();
                    StartQuiz();
                }
                else
                {
                    MessageBox.Show("Please select a theme.", "Warning", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                }
            };
            selectButton.Location = new Point(10, 50);
            Controls.Add(selectButton);

            Button backButton = new Button();
            backButton.Text = "Назад в главное меню";
            backButton.Click += (s, e) => ShowMainMenu();
            backButton.Location = new Point(10, 90);
            Controls.Add(backButton);
        }

        private List<string> GetThemesFromXml()
        {
            List<string> themes = new List<string>();
            XmlNodeList themeNodes = xmlDoc.SelectNodes("//theme");
            foreach (XmlNode themeNode in themeNodes)
            {
                themes.Add(themeNode.Attributes["name"].Value);
            }
            return themes;
        }


        private void StartQuiz()
        {
            Controls.Clear();
            currentLevel = 1;  // Начинаем с первого уровня
            score = 0;
            LoadQuestionsForLevel();
            ShowQuestion();
            StartTimer();
        }

        private void ShowAdminPanel()
        {
            // TODO: Реализовать окно администрирования (отдельная форма лучше)
            AdminForm adminForm = new AdminForm(xmlFilePath);  // Передаем путь к файлу
            adminForm.ShowDialog(); // Модальное окно
            LoadXmlData(); // Обновляем данные после закрытия админки
        }

        private void LoadQuestionsForLevel()
        {
            questions.Clear();
            string xpath = $"//theme[@name='{currentTheme}']/level[@difficulty='{currentLevel}']/question";
            XmlNodeList questionNodes = xmlDoc.SelectNodes(xpath);

            List<XmlNode> questionNodeList = questionNodes.Cast<XmlNode>().ToList();

            // Перемешиваем вопросы
            questionNodeList = questionNodeList.OrderBy(x => random.Next()).ToList();

            // Берем только первые 5 вопросов (или меньше, если вопросов меньше 5)
            questionNodeList = questionNodeList.Take(5).ToList();


            foreach (XmlNode questionNode in questionNodeList)
            {
                Question question = new Question();
                question.ImagePath = questionNode.Attributes["image"]?.Value;
                question.Hint = questionNode.Attributes["hint"]?.Value;
                question.Text = questionNode.SelectSingleNode("text")?.InnerText;

                // Получаем ID вопроса
                if (questionNode.Attributes["id"] != null)
                {
                    question.Id = int.Parse(questionNode.Attributes["id"].Value);
                }
                else
                {
                    question.Id = -1; // Или другое значение по умолчанию, если ID нет
                }

                XmlNodeList answerNodes = questionNode.SelectNodes("answer");
                int correctAnswerIndex = -1;
                for (int i = 0; i < answerNodes.Count; i++)
                {
                    string answerText = answerNodes[i].InnerText;
                    question.Answers.Add(answerText);
                    if (answerNodes[i].Attributes["correct"]?.Value == "true")
                    {
                        correctAnswerIndex = i;
                    }
                }

                question.CorrectAnswerIndex = correctAnswerIndex;
                questions.Add(question);
            }

            currentQuestionIndex = 0; // Начинаем с первого вопроса
        }

        private void ShowQuestion()
        {
            Controls.Clear();

            if (currentQuestionIndex >= questions.Count)
            {
                ShowResults();
                return;
            }

            Question currentQuestion = questions[currentQuestionIndex];

            // Изображение
            PictureBox pictureBox = new PictureBox();
            try
            {
                pictureBox.Image = Image.FromFile(Path.Combine("images", currentQuestion.ImagePath)); // Предполагается, что изображения в папке "images"
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error loading image: " + ex.Message, "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                pictureBox.Image = null; // Или изображение по умолчанию
            }
            pictureBox.SizeMode = PictureBoxSizeMode.Zoom;
            pictureBox.Size = new Size(200, 150);
            pictureBox.Location = new Point(10, 10);
            Controls.Add(pictureBox);

            // Подсказка (label или button)
            Label hintLabel = new Label();
            hintLabel.Text = "Подсказка: " + currentQuestion.Hint;
            hintLabel.Location = new Point(220, 10);
            Controls.Add(hintLabel);

            // Вопрос
            Label questionLabel = new Label();
            questionLabel.Text = currentQuestion.Text;
            questionLabel.Location = new Point(10, 170);
            Controls.Add(questionLabel);

            // Выпадающий список с вариантами ответов
            ComboBox answerComboBox = new ComboBox();
            answerComboBox.Items.AddRange(currentQuestion.Answers.ToArray());
            answerComboBox.Location = new Point(10, 200);
            Controls.Add(answerComboBox);

            // Кнопка "Ответить"
            Button answerButton = new Button();
            answerButton.Text = "Ответить";
            answerButton.Click += (s, e) => CheckAnswer(answerComboBox.SelectedIndex);
            answerButton.Location = new Point(10, 240);
            Controls.Add(answerButton);

            // Таймер
            Label timerLabel = new Label();
            timerLabel.Text = "Осталось времени: " + timeRemaining;
            timerLabel.Location = new Point(220, 170);
            Controls.Add(timerLabel);

            // Индикатор прогресса
            Label progressLabel = new Label();
            progressLabel.Text = $"Вопрос {currentQuestionIndex + 1} из {questions.Count}";
            progressLabel.Location = new Point(220, 200);
            Controls.Add(progressLabel);
        }

        private void CheckAnswer(int selectedAnswerIndex)
        {
            StopTimer();

            Question currentQuestion = questions[currentQuestionIndex];
            if (selectedAnswerIndex == currentQuestion.CorrectAnswerIndex)
            {
                score += 100 / questions.Count; // 100 баллов за уровень
                MessageBox.Show("Правильно!", "Результат", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            else
            {
                MessageBox.Show("Неправильно! Правильный ответ: " + currentQuestion.Answers[currentQuestion.CorrectAnswerIndex], "Результат", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }

            currentQuestionIndex++;
            if (currentQuestionIndex < questions.Count)
            {
                ShowQuestion();
                StartTimer();
            }
            else
            {
                ShowResults();
            }
        }

        private void ShowResults()
        {
            Controls.Clear();

            Label scoreLabel = new Label();
            scoreLabel.Text = "Ваш счет: " + score;
            scoreLabel.Location = new Point(10, 10);
            Controls.Add(scoreLabel);

            Button nextLevelButton = new Button();
            nextLevelButton.Text = "Перейти на следующий уровень";
            nextLevelButton.Click += NextLevelButton_Click;
            nextLevelButton.Location = new Point(10, 50);

            Button backToMenuButton = new Button();
            backToMenuButton.Text = "Вернуться в главное меню";
            backToMenuButton.Click += (s, e) => ShowMainMenu();
            backToMenuButton.Location = new Point(10, 90);
            Controls.Add(backToMenuButton);

            // Проверка проходного балла
            if (score >= 80 && currentLevel < 3) // Предполагаем 3 уровня
            {
                Controls.Add(nextLevelButton);
            }
            else
            {
                if(currentLevel == 3 && score >= 80)
                {
                    MessageBox.Show("Поздравляем! Вы прошли все уровни!", "Поздравления!", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
                else
                {
                     MessageBox.Show("Вы не набрали достаточно баллов для перехода на следующий уровень.", "Результат", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            }
        }


        private void NextLevelButton_Click(object sender, EventArgs e)
        {
            currentLevel++;
            StartQuiz();
        }

        private void StartTimer()
        {
            timeRemaining = 60;
            timer.Start();
        }

        private void StopTimer()
        {
            timer.Stop();
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            timeRemaining--;
            // Обновляем Label таймера
            Control timerLabel = Controls.OfType<Label>().FirstOrDefault(lbl => lbl.Text.StartsWith("Осталось времени")); //Нашли таймер
            if (timerLabel != null) timerLabel.Text = "Осталось времени: " + timeRemaining;


            if (timeRemaining <= 0)
            {
                StopTimer();
                MessageBox.Show("Время вышло!", "Время", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                CheckAnswer(-1); // Считаем как неправильный ответ
            }
        }
    }
}
