from tkinter import *
import random
import copy
from PIL import Image, ImageTk

# creating app main window
game = Tk()

game.title('This Is War!')
game.iconbitmap('./card-icon.ico')
game.geometry('900x600')
game.configure(background="black")

computer = []
player = []
deck = []
removalList = []


def resizeCards(card):
    cardImg = Image.open(card)
    resizedCard = cardImg.resize((130,180))

    global finalCard 
    finalCard = ImageTk.PhotoImage(resizedCard)

    return finalCard


def startGame():
    global computer, player, deck
    computer.clear()
    player.clear()

    figures = ["clubs", "diamonds", "hearts", "spades"]
    #jack-11, queen-12, king-13, ace-14
    values = range(2,15)

    for figure in figures:
        for value in values:
            deck.append(f'{value}_{figure}')
    
    #split deck
    while len(computer) < 26 and len(player) < 26:
        computerCard = random.choice(deck)
        deck.remove(computerCard)
        computer.append(computerCard)

        playerCard = random.choice(deck) 
        deck.remove(playerCard) 
        player.append(playerCard) 

    print('------------------------------------------------------------------------------------------')
    print('Computer cards:')
    print(computer, len(computer))
    print('------------------------------------------------------------------------------------------')
    print('Player cards:')
    print(player, len(player))
    print('------------------------------AFTER SHUFFLE. PRESS GET CARD. SEE SELECTED CARDS AND COMPARE----------------------------------------------')

    #start card
    global startImage
    startImage = resizeCards('./cards/0_back_of_card.png')
    computerLabel.config(image = startImage)
    playerLabel.config(image = startImage)

    computerCards.config(text="You have {} cards.".format(len(computer)))
    playerCards.config(text="You have {} cards.".format(len(player)))

def war(computerCardNo, playerCardNo):
    removedComputerCards = []
    removedPlayerCards = []
    global computer
    global player

    if computerCardNo == playerCardNo:
        removedComputerCards.append(computer[0])
        removedComputerCards.append(computer[1])
        del computer[0:2]

        removedPlayerCards.append(player[0])
        removedPlayerCards.append(player[1])
        del player[0:2]

        computerCardNo = int(computer[0].split("_")[0])
        playerCardNo = int(player[0].split("_")[0])
    
    if computer[0] > player[0]:
        winner = 'c'
    else:
        winner = 'p'
    
    totalCards = removedComputerCards + removedPlayerCards

    return winner, totalCards, computerCardNo, playerCardNo


def warWithOneCard(indicator):
    print('INDICATOR:', indicator)
    global computer, player

    removedOtherCards = []

    if indicator == 'c':
        indicatorList = copy.deepcopy(computer)
        otherList = copy.deepcopy(player)
    else:
        indicatorList = copy.deepcopy(player)
        otherList = copy.deepcopy(computer)

    removedOtherCards.append(otherList[0])
    removedOtherCards.append(otherList[1])
    del otherList[0:2]
    otherCardNo = int(otherList[0].split("_")[0])

    if indicatorList[0] > otherList[0]:
        winner = indicator
    else:
        winner = 'o'
    
    print('other list', removedOtherCards)

    return winner, removedOtherCards, otherList, otherCardNo


def calculatingScorePerRound(computerCard, playerCard):
    global computer
    global player
    global removalList

    computerCardNo = int(computerCard.split("_")[0])
    playerCardNo = int(playerCard.split("_")[0])

    #tie
    if computerCardNo == playerCardNo:
        #using the rule with 2 cards down (one face down, one face up)

        scoreLabel.config(text="It's a tie. It means WAR!")
        #normal case: computer and player have enough cards
        if len(computer) > 2 and len(player) > 2:
            winner, totalCards, newComputer, newPlayer = war(computerCardNo, playerCardNo)
            for el in totalCards:
                removalList.append(el)

            print('removal list:', removalList)
        
        #case 1: computer has one card left to put down
        if len(computer) < 2 and len(player) > 2:
            winner, removeOtherCards, otherList, newIndicator = warWithOneCard('c')
            player = copy.deepcopy(otherList)
            for el in removeOtherCards:
                removalList.append(el)
            if newIndicator != computer[0]:
                if winner == 'c':
                    for el in removalList:
                        computer.append(el)
                else:
                    finalScore.config(text='You won! Congrats!!!')
                    return
        
        #case 2: player has one card left to put down
        if len(player) < 2 and len(computer) > 2:
            winner, removeOtherCards, otherList, newIndicator = warWithOneCard('p')
            computer = copy.deepcopy(otherList)
            if newIndicator != computer[0]:
                if winner == 'p':
                    for el in removeOtherCards:
                        player.append(el)
                else:
                    finalScore.config(text='Computer won! You lost! Game over.')

    #player wins round
    if playerCardNo > computerCardNo:
        scoreLabel.config(text="You win this round!")
        if len(removalList) != 0:
            for el in removalList:
                player.append(el)
        player.append(player[0])
        player.remove(player[0])
        player.append(computer[0])
        computer.remove(computer[0])
        computerCards.config(text="You have {} cards.".format(len(computer)))
        playerCards.config(text="You have {} cards.".format(len(player)))
        removalList.clear()

    #computer wins round
    elif playerCardNo < computerCardNo:
        scoreLabel.config(text="Computer wins this round!")
        if len(removalList) != 0:
            for el in removalList:
                computer.append(el)
        computer.append(computer[0])
        computer.remove(computer[0])
        computer.append(player[0])
        player.remove(player[0])
        computerCards.config(text="You have {} cards.".format(len(computer)))
        playerCards.config(text="You have {} cards.".format(len(player)))
        removalList.clear()


def getCard():
    try:

        #put card on table 
        computerCard = computer[0]
        print('Selected card for computer:', computerCard)
        global computerImage
        computerImage = resizeCards(f'./cards/{computerCard}.png')
        computerLabel.config(image = computerImage)

        playerCard = player[0]
        print('Selected card for player:', playerCard)
        global playerImage
        playerImage = resizeCards(f'./cards/{playerCard}.png')
        playerLabel.config(image = playerImage)
        print('------------------------------------ LISTS AFTER CALCULATING SCORE -----------------------------------------')

        #update score
        calculatingScorePerRound(computerCard, playerCard)
        print('Computer cards:')
        print(computer)
        print('Total cards Computer:', len(computer))
        print('------------------------------------------------------------------------------------------')
        print('Player cards:')
        print(player)
        print('Total cards Player:', len(player))
        print('------------------------------------------------------------------------------------------')

    except:

        #player wins
        if len(player) == 52:
            finalScore.config(text='You won! Congrats!!!')
        #computer wins
        elif len(player) == 0:
            finalScore.config(text='Computer won! You lost! Game over.')


warFrame = Frame(game, bg="black")
warFrame.pack(pady=35)

#frames for cards
computerFrame = LabelFrame(warFrame, text="Computer", bd=0)
computerFrame.grid(row=0, column=0, padx=30, ipadx=20)
playerFrame = LabelFrame(warFrame, text="Player", bd=0)
playerFrame.grid(row=0, column=2, ipadx=20)

#cards in frames 
computerLabel = Label(computerFrame, text="")
computerLabel.pack(pady=20)
playerLabel = Label(playerFrame, text="")
playerLabel.pack(pady=20)

#score label tie/win
scoreLabel = Label(warFrame, text="", bg='black', fg='white')
scoreLabel.grid(row=1, column=1, padx=40, ipadx=40, pady=20)

#buttons
buttonFrame = LabelFrame(game, bg='black', bd=0)
buttonFrame.pack(pady=20)


computerCards = Button(buttonFrame, text="", bg='black', font=(SOLID,14), fg='white', bd=0)
computerCards.grid(row=2, column=0, pady=40, ipadx=20)


playerCards = Button(buttonFrame, text="",  bg='black', font=(SOLID,14), fg='white', bd=0)
playerCards.grid(row=2, column=2, ipadx=20)


shuffleButton = Button(buttonFrame, text="Shuffle Cards", font=(SOLID,14), command=startGame)
shuffleButton.grid(row=0, column=1,padx=40, ipadx=40)

getCardButton = Button(buttonFrame, text="Get Card", font=(SOLID,14), bg= 'green', command=getCard)
getCardButton.grid(row=2, column=1, ipadx=20)

#final score --> you win or you lose the game
finalScore = Label(warFrame, text="", bg="black", font=14, fg='red')
finalScore.grid(row=2, column=1, padx=40, ipadx=40, pady=20)

startGame()
game.mainloop()