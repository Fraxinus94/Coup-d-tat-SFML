#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Audio.hpp>

const float PI = 3.14159265358979323846f;

struct Bullet 
{
    sf::Sprite sprite;
    sf::Vector2f velocity;

    Bullet(sf::Texture& texture) { sprite.setTexture(texture); };
};

sf::Text createText(std::string string, sf::Font& font, unsigned int fontSize, sf::Color color, float posx, float posy)
{
    sf::Text t;
    t.setString(string);
    t.setFont(font);
    t.setCharacterSize(fontSize);
    t.setFillColor(color);
    t.setPosition(posx, posy);
    t.setOrigin(t.getGlobalBounds().left * 0.5f, t.getGlobalBounds().top * 0.5f);

    return t;
}

int main()
{
    std::string title = "Coup d'état";
    sf::RenderWindow window(sf::VideoMode(720, 480), title);

    window.setFramerateLimit(60);

    sf::Vector2f center(window.getSize().x * 0.5f, window.getSize().y * 0.5f);

    sf::Texture mainTexture;
    mainTexture.loadFromFile("./art/menu.png");
    sf::Sprite mainSprite;
    mainSprite.setTexture(mainTexture);

    sf::Texture backgroundTexture;
    backgroundTexture.loadFromFile("./art/map.png");
    sf::Sprite backgroundSprite;
    backgroundSprite.setTexture(backgroundTexture);

    sf::Texture coopTexture;
    coopTexture.loadFromFile("./art/coop.png");
    sf::Sprite coopSprite;
    coopSprite.setTexture(coopTexture);
    coopSprite.setOrigin(coopSprite.getPosition().x + coopSprite.getGlobalBounds().width * 0.5f, coopSprite.getPosition().y + coopSprite.getGlobalBounds().height * 0.5f);
    coopSprite.setPosition(center.x, center.y);

    sf::Texture playerTexture;
    playerTexture.loadFromFile("./art/beanie.png");
    sf::Sprite playerSprite;
    playerSprite.setTexture(playerTexture);
    playerSprite.setOrigin(playerSprite.getPosition().x + playerSprite.getGlobalBounds().width * 0.5f, playerSprite.getPosition().y + playerSprite.getGlobalBounds().height * 0.5f);
    playerSprite.setPosition(center.x, center.y);
    float rotationSpeed = 2.0f;
    float moveSpeed = 2.0f;

    sf::Texture beanTexture;
    beanTexture.loadFromFile("./art/bean.png");
    sf::Sprite beanSprite;
    beanSprite.setTexture(beanTexture);
    std::vector<Bullet> bullets;
    float bulletSpeed = 5.0f;

    sf::Texture enemyTexture;
    enemyTexture.loadFromFile("./art/chicken.png");
    sf::Sprite enemySprite;
    enemySprite.setTexture(enemyTexture);
    enemySprite.setOrigin(enemySprite.getPosition().x + enemySprite.getGlobalBounds().width * 0.5f, enemySprite.getPosition().y + enemySprite.getGlobalBounds().height * 0.5f);

    int numEnemies = 10;
    float radius = 400.0f;
    std::vector<sf::Sprite> enemies(numEnemies, enemySprite);
    for (int i = 0; i < numEnemies; ++i)
    {
        float angle = 2 * PI * i / numEnemies;
        sf::Vector2f position = center + sf::Vector2f(std::cos(angle), std::sin(angle)) * radius;
        enemies[i].setPosition(position);
    }
    float speed = 0.5f;

    std::vector<int> bulletsToRemove;
    std::vector<int> enemiesToRemove;

    sf::Clock shootClock;
    sf::Time shootCooldown = sf::milliseconds(300);

    sf::Clock autoShootClock1;
    sf::Clock autoShootClock2;
    //sf::Time autoShootCooldown = sf::milliseconds(1000);
    std::vector<sf::Time> autoShootTimes = { sf::milliseconds(1000) };

    unsigned int score = 0;
    unsigned int highScore = 0;
    int playerLives = 3;
    int currentWave = 1;
    int bossHitCount = 0;
    int bossHealth = 5;
    int money = 900000;

    bool paused = false;

    sf::Font font{};
    if (!font.loadFromFile("./fonts/adam.otf")) { }

    sf::Text menuText = createText("Coup d'état", font, 48, sf::Color::Red, center.x + 80.0f, center.y);
    bool onMenu = true;

    sf::Text spaceText = createText("Press 'Spacebar' to start", font, 18, sf::Color::Red, center.x + 140.0f, center.y + 160.0f);
    int spaceAlpha = 255;
    bool isIncreasing = false;

    sf::Text moneyText = createText("Eggs: ", font, 24, sf::Color::White, 0.0f, 90.0f);
    moneyText.setOrigin(0.0f, 0.0f);

    sf::Text scoreText = createText("Score: ", font, 24, sf::Color::White, 0.0f, 40.0f);
    scoreText.setOrigin(0.0f, 0.0f);

    sf::Text livesText = createText("Lives: ", font, 24, sf::Color::White, 0.0f, 30.0f);
    livesText.setOrigin(0.0f, 0.0f);

    sf::Text highScoreText = createText("Highscore: ", font, 24, sf::Color::White, 0.0f, 10.0f);
    highScoreText.setOrigin(0.0f, 0.0f);

    sf::Text gameOverText = createText("Game Over", font, 48, sf::Color::Red, 0.0f, 40.0f);
    bool gameOver = false;

    sf::Text rText = createText("Press 'R' to restart", font, 18, sf::Color::Red, 0.0f, 0.0f);
    int rAlpha = 255;

    sf::Text pausedText = createText("Paused", font, 48, sf::Color::Red, 0.0f, 0.0f);

    sf::Text upgradeOneText = createText("Shoot Speed LvL: ", font, 18, sf::Color::Red, 0.0f, 0.0f);
    int shootSpeedLevel = 0;

    sf::Text upgradeTwoText = createText("Auto Shoot LvL: ", font, 18, sf::Color::Red, 0.0f, 0.0f);
    int autoShootLevel = 0;
    int autoShootLayerLevel = 0;
    int shotCount = 0;
    int autoCount = 0;
    //sf::Time shootSpeed = sf::milliseconds(1000);

    sf::Text upgradeThreeText = createText("Bullet Damage LvL: ", font, 18, sf::Color::Red, 0.0f, 0.0f);
    int shootDamageLevel = 1;

    sf::Text costTextOne = createText("Cost: ", font, 18, sf::Color::Red, 0.0f, 0.0f);

    sf::Text costTextTwo = createText("Cost: ", font, 18, sf::Color::Red, 0.0f, 0.0f);

    sf::Text costTextThree = createText("Cost: ", font, 18, sf::Color::Red, 0.0f, 0.0f);

    sf::Text buyTextOne = createText("Buy", font, 18, sf::Color::Red, 0.0f, 0.0f);

    sf::Text buyTextTwo = createText("Buy", font, 18, sf::Color::Red, 0.0f, 0.0f);

    sf::Text buyTextThree = createText("Buy", font, 18, sf::Color::Red, 0.0f, 0.0f);

    sf::RectangleShape buttonOne(sf::Vector2f(36.0f, 30.0f));
    sf::RectangleShape buttonTwo(sf::Vector2f(36.0f, 30.0f));
    sf::RectangleShape buttonThree(sf::Vector2f(36.0f, 30.0f));

    sf::SoundBuffer bufferMenu;
    bufferMenu.loadFromFile("./sfx/menu.mp3");
    sf::Sound menuSound;
    menuSound.setBuffer(bufferMenu);

    menuSound.play();
    menuSound.setLoop(true);

    sf::SoundBuffer bufferBackground;
    bufferBackground.loadFromFile("./sfx/background_music.mp3");
    sf::Sound chickenBackground;
    chickenBackground.setBuffer(bufferBackground);
    bool gameMusicStarted = false;

    sf::SoundBuffer bufferKill;
    bufferKill.loadFromFile("./sfx/killchicken.mp3");
    sf::Sound chickenKill;
    chickenKill.setBuffer(bufferKill);

    sf::SoundBuffer bufferTender;
    bufferTender.loadFromFile("./sfx/tenders.mp3");
    sf::Sound chickenTender;
    chickenTender.setBuffer(bufferTender);

    sf::SoundBuffer bufferShoot;
    bufferShoot.loadFromFile("./sfx/shoot.wav");
    sf::Sound shootSound;
    shootSound.setBuffer(bufferShoot);

    while (window.isOpen())
    {
        if (gameMusicStarted)
        {
            chickenBackground.play();
            chickenBackground.setLoop(true);
            gameMusicStarted = false; // Just a hacky check
        }

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (!onMenu && !gameOver && event.type == event.KeyPressed && sf::Keyboard::isKeyPressed(sf::Keyboard::Key::P))
            {
                if (!paused)
                {
                    paused = true;
                }

                else if (paused)
                {
                    paused = false;
                }
            }

            if (paused)
            {
                if (event.type == event.MouseButtonPressed && sf::Mouse::isButtonPressed(sf::Mouse::Left))
                {
                    sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                    sf::Vector2f mouseCoords = window.mapPixelToCoords(mousePosition);

                    sf::CircleShape pos(1.0f);
                    pos.setPosition(mouseCoords);

                    if (buttonOne.getGlobalBounds().intersects(pos.getGlobalBounds()))
                    {
                        if (money >= (int)(((shootSpeedLevel + 1) * 5) * 0.75f))
                        {
                            money = money - (int)(((shootSpeedLevel + 1) * 5) * 0.75f);

                            shootSpeedLevel++;

                            shootCooldown -= sf::milliseconds((int)(0.2f * shootSpeedLevel));
                        }
                        pos.setPosition(0.0f, 0.0f);
                    }

                    if (buttonTwo.getGlobalBounds().intersects(pos.getGlobalBounds()))
                    {
                        if (money >= (int)(((autoShootLevel + 1) * 5) * 0.75f))
                        {
                            money = money - (int)(((autoShootLevel + 1) * 5) * 0.75f);

                            if (autoShootLevel % 16 == 0)
                            {
                                autoShootLayerLevel++;
                                autoShootTimes.push_back(sf::milliseconds(1000));
                            }

                            autoShootLevel++;

                            if (autoShootLevel % 5 == 0)
                            {
                                for (auto& shootSpeed : autoShootTimes)
                                {
                                    shootSpeed -= sf::milliseconds(50);
                                }
                            }
                            else
                            {
                                shotCount++;
                            }
                        }

                        pos.setPosition(0.0f, 0.0f);
                    }

                    if (buttonThree.getGlobalBounds().intersects(pos.getGlobalBounds()))
                    {
                        if (money >= (int)(((shootDamageLevel + 1) * 5) * 0.75f))
                        {
                            money = money - (int)(((shootDamageLevel + 1) * 5) * 0.75f);

                            shootDamageLevel++;
                        }

                        pos.setPosition(0.0f, 0.0f);
                    }
                }
            }

        }

        if (onMenu)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                menuSound.stop();
                menuSound.setLoop(false);
                onMenu = false;
                gameMusicStarted = true;
            }
        }

        if (gameOver && sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        {
            chickenTender.stop();
            chickenBackground.play();
            chickenBackground.setLoop(true);
            numEnemies = 10;
            speed = 0.5f;
            score = 0;
            playerLives = 3;
            currentWave = 1;
            money = 0;
            bossHealth = 5;
            shootDamageLevel = 1;
            autoShootLevel = 0;
            autoShootLayerLevel = 0;
            shootSpeedLevel = 0;
            //shootSpeed = sf::milliseconds(1000);
            shootCooldown = sf::milliseconds(300);
            shotCount = 0;
            playerSprite.setPosition(center.x, center.y);
            playerSprite.setRotation(0.0f);
            gameOver = false;
            autoShootTimes.clear();
            autoShootTimes.push_back(sf::milliseconds(1000));
        }

        if (!gameOver && !onMenu && !paused)
        {
            sf::Vector2f movement(0.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            {
                movement.y -= moveSpeed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            {
                movement.y += moveSpeed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            {
                movement.x -= moveSpeed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            {
                movement.x += moveSpeed;
            }
            playerSprite.move(movement);

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && shootClock.getElapsedTime() >= shootCooldown)
            {
                shootSound.play();

                sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
                sf::Vector2f mouseCoords = window.mapPixelToCoords(mousePosition);

                sf::Vector2f playerPosition = playerSprite.getPosition();
                float dx = mouseCoords.x - playerPosition.x;
                float dy = mouseCoords.y - playerPosition.y;
                float angle = std::atan2(dy, dx) * 180 / PI;

                Bullet bullet(beanTexture);
                bullet.sprite.setPosition(playerPosition);

                float bulletSpeed = 5.0f;
                bullet.velocity.x = std::cos(angle * PI / 180.0f) * bulletSpeed;
                bullet.velocity.y = std::sin(angle * PI / 180.0f) * bulletSpeed;

                bullets.push_back(bullet);
                shootClock.restart();
            }
        }

        if (!onMenu && !paused && !gameOver)
        {
            sf::Vector2f playerPos = playerSprite.getPosition();
            if (playerPos.x < 0.0f)
            {
                playerSprite.setPosition(0.0f, playerPos.y);
            }
            if (playerPos.x > center.x * 2.0f)
            {
                playerSprite.setPosition(center.x * 2.0f, playerPos.y);
            }
            if (playerPos.y < 0.0f)
            {
                playerSprite.setPosition(playerPos.x, 0.0f);
            }
            if (playerPos.y > center.y * 2.0f)
            {
                playerSprite.setPosition(playerPos.x, center.y * 2.0f);
            }

            sf::Vector2f playerPosition = playerSprite.getPosition();
            for (auto& enemy : enemies)
            {

                sf::Vector2f direction = playerPosition - enemy.getPosition();
                float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
                if (length != 0)
                {
                    direction /= length;

                    enemy.move(direction * speed);

                    if (direction.x > 0.0f)
                    {
                        enemy.setScale(-1.0f, 1.0f);
                    }

                    if (direction.x < 0.0f)
                    {
                        enemy.setScale(1.0f, 1.0f);
                    }
                }
            }

            //sf::Time time1 = autoShootClock1.getElapsedTime();
            if (autoShootLevel > 0)
            {
                if (autoShootClock1.getElapsedTime() > autoShootTimes[0])
                {
                    for (int i = 0; i < autoShootLayerLevel; i++)
                    {
                        sf::Time time2 = autoShootClock2.getElapsedTime();
                        //printf("time2: %i\n", time1.asMilliseconds());
                        //sf::Time time = autoShootClock.getElapsedTime();
                        if (autoShootClock2.getElapsedTime() >= autoShootTimes[i])
                        {
                            shootSound.play();

                            for (int j = 0; j < shotCount; j++)
                            {
                                sf::Vector2f playerPosition = playerSprite.getPosition();
                                float angle = (j % 16) * 22.5f;

                                Bullet bullet(beanTexture);
                                bullet.sprite.setPosition(playerPosition);

                                float bulletSpeed = 5.0f;
                                bullet.velocity.x = std::cos(angle * PI / 180.0f) * bulletSpeed;
                                bullet.velocity.y = std::sin(angle * PI / 180.0f) * bulletSpeed;

                                bullets.push_back(bullet);
                            }

                            autoShootClock2.restart();
                        }
                    }
                    autoShootClock1.restart();
                }
            }

            for (auto& bullet : bullets)
            {
                bullet.sprite.move(bullet.velocity);
            }

            bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [&](const Bullet& bullet)
                {
                    return bullet.sprite.getPosition().x < 0 ||
                        bullet.sprite.getPosition().y < 0 ||
                        bullet.sprite.getPosition().x > window.getSize().x ||
                        bullet.sprite.getPosition().y > window.getSize().y;
                }), bullets.end());

            for (size_t i = 0; i < bullets.size(); ++i)
            {
                for (size_t j = 0; j < enemies.size(); ++j)
                {
                    if (bullets[i].sprite.getGlobalBounds().intersects(enemies[j].getGlobalBounds()))
                    {
                        bulletsToRemove.push_back((int)i);

                        if (currentWave % 5 == 0)
                        {
                            bossHitCount += shootDamageLevel;

                            if (bossHitCount >= bossHealth)
                            {
                                enemiesToRemove.push_back((int)j);
                                bossHitCount = 0;

                                int roundMultiplier = (int)std::round(currentWave / 5.0f);
                                bossHealth = (int)((5 * ((roundMultiplier > 1) ? roundMultiplier : 1)) * 0.75f);
                            }
                            else
                            {
                                break;
                            }
                        }
                        enemiesToRemove.push_back((int)j);
                    }
                }
            }

            for (size_t i = 0; i < enemies.size(); ++i)
            {
                if (playerSprite.getGlobalBounds().intersects(enemies[i].getGlobalBounds()))
                {
                    playerLives -= 1;
                    score -= 1;
                    enemiesToRemove.push_back((int)i);
                }
            }

            std::sort(bulletsToRemove.rbegin(), bulletsToRemove.rend());
            bulletsToRemove.erase(std::unique(bulletsToRemove.begin(), bulletsToRemove.end()), bulletsToRemove.end());

            std::sort(enemiesToRemove.rbegin(), enemiesToRemove.rend());
            enemiesToRemove.erase(std::unique(enemiesToRemove.begin(), enemiesToRemove.end()), enemiesToRemove.end());

            for (int index : bulletsToRemove)
            {
                bullets.erase(bullets.begin() + index);
            }
            bulletsToRemove.clear();

            for (int index : enemiesToRemove)
            {
                if (currentWave % 5 == 0)
                {
                    score += 5;
                    money += (int)(5 + (currentWave * 0.1f));
                }
                else
                {
                    score += 1;
                    money += (int)(1 + (currentWave * 0.1f));
                }

                enemies.erase(enemies.begin() + index);
                chickenKill.play();
            }
            enemiesToRemove.clear();

            if (enemies.empty() && !gameOver)
            {
                numEnemies += 1;
                currentWave += 1;
                speed *= 1.05f;

                if (currentWave % 5 == 0)
                {
                    float angle = 2 * PI * 0 / 1;
                    sf::Vector2f position = center + sf::Vector2f(std::cos(angle), std::sin(angle)) * radius;
                    enemySprite.setPosition(position);
                    enemies.push_back(enemySprite);
                }
                else
                {
                    for (int i = 0; i < numEnemies; ++i)
                    {
                        float angle = 2 * PI * i / numEnemies;
                        sf::Vector2f position = center + sf::Vector2f(std::cos(angle), std::sin(angle)) * radius;
                        enemySprite.setPosition(position);
                        enemies.push_back(enemySprite);
                    }
                }
            }
        }

        if (score > highScore)
        {
            highScore = score;
        }

        moneyText.setString("Eggs: " + std::to_string(money));
        scoreText.setString("Score: " + std::to_string(score));
        livesText.setString("Lives: " + std::to_string(playerLives));
        highScoreText.setString("Highscore: " + std::to_string(highScore));
        upgradeOneText.setString("Shoot Speed LvL: " + std::to_string(shootSpeedLevel));
        upgradeTwoText.setString("Auto Shoot LvL: " + std::to_string(autoShootLevel));
        upgradeThreeText.setString("Bullet Damage LvL: " + std::to_string(shootDamageLevel));

        if (playerLives <= 0 && !gameOver) 
        {
            chickenBackground.stop();
            chickenBackground.setLoop(false);
            enemies.clear();
            bullets.clear();
            enemiesToRemove.clear();
            bulletsToRemove.clear();
            gameOver = true;
            chickenTender.play();
        }

        window.clear();

        if (onMenu)
        {
            window.draw(mainSprite);

            window.draw(menuText);

            if (spaceAlpha >= 255)
            {
                isIncreasing = false;
            }
            if (spaceAlpha <= 0)
            {
                isIncreasing = true;
            }
            if (isIncreasing)
            {
                spaceAlpha += 3;
            }
            if (!isIncreasing)
            {
                spaceAlpha -= 3;
            }

            spaceText.setFillColor(sf::Color(255, 0, 0, spaceAlpha));
            window.draw(spaceText);
        }

        if (!gameOver && !onMenu && !paused)
        {
            window.draw(backgroundSprite);

            window.draw(coopSprite);

            window.draw(playerSprite);

            for (const auto& bullet : bullets)
            {
                window.draw(bullet.sprite);
            }

            for (const auto& enemy : enemies)
            {

                if (currentWave % 5 == 0)
                {
                    enemies[0].setScale(2.5f, 2.5f);
                }
                window.draw(enemy);
            }

            scoreText.setFillColor(sf::Color::White);
            scoreText.setPosition(0.0f, 60.0f);
            scoreText.setOrigin(0.0f, 0.0f);
            window.draw(scoreText);

            window.draw(livesText);

            highScoreText.setFillColor(sf::Color::White);
            highScoreText.setPosition(0.0f, 0.0f);
            highScoreText.setOrigin(0.0f, 0.0f);
            window.draw(highScoreText);

            moneyText.setFillColor(sf::Color::White);
            moneyText.setPosition(0.0f, 90.0f);
            moneyText.setOrigin(0.0f, 0.0f);
            window.draw(moneyText);
        } 

        if (gameOver && !onMenu && !paused)
        {
            window.draw(mainSprite);

            gameOverText.setPosition(center.x, center.y * 0.75f);
            gameOverText.setOrigin(gameOverText.getGlobalBounds().width * 0.5f, gameOverText.getGlobalBounds().top * 0.5f);
            gameOverText.setFillColor(sf::Color::Red);
            window.draw(gameOverText);

            scoreText.setFillColor(sf::Color::Red);
            scoreText.setPosition(center.x, center.y * 1.50f);
            scoreText.setOrigin(scoreText.getGlobalBounds().width * 0.5f, scoreText.getGlobalBounds().top * 0.5f);
            window.draw(scoreText);

            highScoreText.setFillColor(sf::Color::Red);
            highScoreText.setPosition(center.x, center.y * 1.35f);
            highScoreText.setOrigin(highScoreText.getGlobalBounds().width * 0.5f, highScoreText.getGlobalBounds().top * 0.5f);
            window.draw(highScoreText);

            if (rAlpha >= 255)
            {
                isIncreasing = false;
            }
            if (rAlpha <= 0)
            {
                isIncreasing = true;
            }
            if (isIncreasing)
            {
                rAlpha += 3;
            }
            if (!isIncreasing)
            {
                rAlpha -= 3;
            }

            rText.setFillColor(sf::Color(255, 0, 0, rAlpha));
            rText.setPosition(center.x + 50.0f, center.y * 1.8f);
            rText.setOrigin(rText.getGlobalBounds().left * 0.5f, rText.getGlobalBounds().top * 0.5f);

            window.draw(rText);
        }

        if (paused)
        {
            moneyText.setFillColor(sf::Color::Red);
            moneyText.setPosition(center.x + 65.0f, center.y * 0.25f);
            moneyText.setOrigin(moneyText.getGlobalBounds().left * 0.5f, moneyText.getGlobalBounds().top * 0.5f);
            window.draw(moneyText);

            upgradeOneText.setPosition(center.x + 65.0f, center.y * 0.5f);
            upgradeOneText.setOrigin(upgradeOneText.getGlobalBounds().left * 0.5f, upgradeOneText.getGlobalBounds().top * 0.5f);
            window.draw(upgradeOneText);

            costTextOne.setPosition(center.x + 65.0f, center.y * 0.65f);
            costTextOne.setOrigin(costTextOne.getGlobalBounds().left * 0.5f, costTextOne.getGlobalBounds().top * 0.5f);
            costTextOne.setString("Cost: " + std::to_string(static_cast<int>(((shootSpeedLevel + 1) * 5) * 0.75f)) + " Eggs");
            window.draw(costTextOne);

            buyTextOne.setPosition(center.x - 65.0f, center.y * 0.57f);
            buyTextOne.setOrigin(buyTextOne.getGlobalBounds().left * 0.5f, buyTextOne.getGlobalBounds().top * 0.5f);
            window.draw(buyTextOne);

            buttonOne.setFillColor(sf::Color::Transparent);
            buttonOne.setOutlineColor(sf::Color::Red);
            buttonOne.setOutlineThickness(2.0f);
            buttonOne.setPosition(center.x - 165.0f, center.y * 0.36f);
            window.draw(buttonOne);

            upgradeTwoText.setPosition(center.x + 65.0f, center.y);
            upgradeTwoText.setOrigin(upgradeTwoText.getGlobalBounds().left * 0.5f, upgradeTwoText.getGlobalBounds().top * 0.5f);
            window.draw(upgradeTwoText);

            costTextTwo.setPosition(center.x + 65.0f, center.y * 1.15f);
            costTextTwo.setOrigin(costTextTwo.getGlobalBounds().left * 0.5f, costTextTwo.getGlobalBounds().top * 0.5f);
            costTextTwo.setString("Cost: " + std::to_string(static_cast<int>(((autoShootLevel + 1) * 5) * 0.75f)) + " Eggs");
            window.draw(costTextTwo);

            buyTextTwo.setPosition(center.x - 65.0f, center.y * 1.07f);
            buyTextTwo.setOrigin(buyTextTwo.getGlobalBounds().left * 0.5f, buyTextTwo.getGlobalBounds().top * 0.5f);
            window.draw(buyTextTwo);

            buttonTwo.setFillColor(sf::Color::Transparent);
            buttonTwo.setOutlineColor(sf::Color::Red);
            buttonTwo.setOutlineThickness(2.0f);
            buttonTwo.setPosition(center.x - 165.0f, center.y * 0.69f);
            window.draw(buttonTwo);

            upgradeThreeText.setPosition(center.x + 65.0f, center.y * 1.5f);
            upgradeThreeText.setOrigin(upgradeThreeText.getGlobalBounds().left * 0.5f, upgradeThreeText.getGlobalBounds().top * 0.5f);
            window.draw(upgradeThreeText);

            costTextThree.setPosition(center.x + 65.0f, center.y * 1.65f);
            costTextThree.setOrigin(costTextThree.getGlobalBounds().left * 0.5f, costTextThree.getGlobalBounds().top * 0.5f);
            costTextThree.setString("Cost: " + std::to_string(static_cast<int>(((shootDamageLevel + 1) * 5) * 0.75f)) + " Eggs");
            window.draw(costTextThree);

            buyTextThree.setPosition(center.x - 65.0f, center.y * 1.57f);
            buyTextThree.setOrigin(buyTextThree.getGlobalBounds().left * 0.5f, buyTextThree.getGlobalBounds().top * 0.5f);
            window.draw(buyTextThree);

            buttonThree.setFillColor(sf::Color::Transparent);
            buttonThree.setOutlineColor(sf::Color::Red);
            buttonThree.setOutlineThickness(2.0f);
            buttonThree.setPosition(center.x - 165.0f, center.y * 1.03f);
            window.draw(buttonThree);
        }

        window.display();
    }

    return 0;
}
