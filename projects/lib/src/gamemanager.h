/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QObject>
#include <QList>
class ChessGame;
class ChessPlayer;
class PlayerBuilder;
class GameThread;


/*!
 * \brief A class for managing chess games and players
 *
 * GameManager can start games in a new thread, run
 * multiple games concurrently, and queue games to be
 * run when a game slot/thread is free.
 *
 * \sa ChessGame, PlayerBuilder
 */
class LIB_EXPORT GameManager : public QObject
{
	Q_OBJECT

	public:
		/*! The mode for starting games. */
		enum StartMode
		{
			/*! The game is started immediately. */
			StartImmediately,
			/*!
			 * The game is added to a queue, and is started when
			 * a game slot becomes free. This could be immediately.
			 */
			Enqueue
		};

		/*! Creates a new game manager. */
		GameManager(QObject* parent = 0);

		/*!
		 * Returns the list of active games.
		 *
		 * Any game that was started (ie. isn't waiting in the queue)
		 * is considered active even if the game has ended. This is
		 * because the game could still be used for analysis, etc.
		 * The game loses its active status only when it's deleted.
		 */
		QList<ChessGame*> activeGames() const;

		/*!
		 * Returns the maximum allowed number of concurrent games.
		 *
		 * The concurrency limit only affects games that are started
		 * in Enqueue mode.
		 *
		 * \sa setConcurrency()
		 */
		int concurrency() const;
		/*!
		 * Sets the concurrency limit to \a concurrency.
		 *
		 * \sa concurrency()
		 */
		void setConcurrency(int concurrency);

		/*!
		 * Removes all future games from the queue, waits for
		 * ongoing games to end, and deletes all idle players.
		 * Emits the finished() signal when done.
		 */
		void finish();

		/*!
		 * Adds a new game to the game manager.
		 *
		 * This function gives control of \a game to the game manager,
		 * which moves the game to its own thread and starts it. Because
		 * the game is played in a separate thread, it must not have a
		 * parent object. When it's not needed anymore, it must be
		 * destroyed with the \a deleteLater() method. Only then will the
		 * game manager free the game slot used by the game.
		 *
		 * Construction of the players is delayed to the moment when the
		 * game starts. If the same builder objects (\a white and \a black)
		 * were used in a previous game, the players are reused instead of
		 * constructing new players.
		 *
		 * If \a mode is StartImmediately, the game starts immediately
		 * even if the number of active games is over the \a concurrency
		 * limit. In \a Enqueue mode the game is started as soon as
		 * a free game slot is available.
		 *
		 * Returns true if successfull (ie. the game was added to the queue
		 * or it was started successfully); otherwise returns false.
		 *
		 * \note If there are still free game slots after starting this
		 * game, the ready() signal is emitted immediately.
		 */
		bool newGame(ChessGame* game,
			     const PlayerBuilder* white,
			     const PlayerBuilder* black,
			     StartMode mode = StartImmediately);

	signals:
		/*! This signal is emitted when a new game starts. */
		void gameStarted(ChessGame* game);
		/*!
		 * This signal is emitted when a game is destroyed.
		 *
		 * Dereferencing the \a game pointer results in undefined
		 * behavior, so this signal should only be used for cleanup.
		 */
		void gameDestroyed(ChessGame* game);
		/*!
		 * This signal is emitted after a game has started
		 * or after a game has ended, if there are free
		 * game slots.
		 *
		 * \note The signal is NOT emitted if a newly freed
		 * game slot can be used by a game that was waiting in
		 * the queue.
		 */
		void ready();
		/*!
		 * This signal is emitted when all games have ended and all
		 * idle players have been deleted. Then the manager can be
		 * safely deleted.
		 */
		void finished();
		/*! This signal redirects the ChessPlayer::debugMessage() signal. */
		void debugMessage(const QString& data);

	private slots:
		void onGameStarted();
		void onThreadReady(GameManager::StartMode mode);
		void onThreadQuit();

	private:
		struct GameEntry
		{
			ChessGame* game;
			const PlayerBuilder* white;
			const PlayerBuilder* black;
		};

		GameThread* getThread(const PlayerBuilder* white,
				      const PlayerBuilder* black);
		bool startGame(const GameEntry& entry, StartMode mode);
		bool startQueuedGame();
		void cleanup();

		bool m_finishing;
		int m_concurrency;
		int m_activeQueuedGameCount;
		QList<GameThread*> m_threads;
		QList<GameEntry> m_gameEntries;
		QList<ChessGame*> m_activeGames;
};

#endif // GAMEMANAGER_H
