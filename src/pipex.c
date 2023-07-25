/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipex.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ymorozov <ymorozov@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/03/23 12:20:49 by ymorozov          #+#    #+#             */
/*   Updated: 2023/07/25 12:45:35 by ymorozov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./../inc/pipex.h"

static int	wait_all(int start, int end);
static int	ft_read_from_pipe(int fd_in, int fd_out);


int	pipex_manager(int *arr_oper, char **argv, char **envp, int *fds_files)
{
	int		i;
	int		j;
	int		fd;
	int		res;
	t_cmd	*cur;

	i = arr_oper[0] - 1;
	j = arr_oper[1];
	fd = fds_files[0];
	cur = (t_cmd *)malloc(sizeof(t_cmd));
	if (cur == NULL)
		return (1);
	while (++i < j)
	{
		res = get_struct(argv[i], envp, cur);
		ft_putstr_fd("pipex_manager: i = ", 2);
		ft_putnbr_fd(i, 2);
		ft_putstr_fd(", j = ", 2);
		ft_putnbr_fd(j, 2);
		ft_putstr_fd(", res = ", 2);
		ft_putnbr_fd(res, 2);
		ft_putstr_fd(", fd = ", 2);
		ft_putnbr_fd(fd, 2);
		ft_putstr_fd("\n", 2);
		if (res == 1)
			return (free(cur), 1);
		else if (res == 2)
		{
			if (pipex_skip(&fd) == 1)
				return (free(cur), 1);
		}
		else
		{
			if (pipex(&fd, cur, envp) == 1)
				return (free_struct(cur), free(cur), 1);
			free_struct(cur);
		}
	}
	free(cur);
	ft_read_from_pipe(fd, fds_files[1]);
	if (wait_all(arr_oper[0], arr_oper[1]) == 1)
		return (close(fd), 1);
	return (close(fd), 0);
}

int	pipex_skip(int *fd)
{
	int		pipefd[2];
	int		fd_rem;
	pid_t	id1;

	fd_rem = open("/dev/null", O_WRONLY);
	if (pipe(pipefd) == -1)
		return (perror("Pipe"), 1);
	id1 = fork();
	if (id1 < 0)
		return (perror("Fork"), 1);
	if (id1 == 0)
	{
		close(pipefd[0]);
		if (dup2(*fd, STDIN_FILENO) < 0 || dup2(pipefd[1], STDOUT_FILENO) < 0)
			return (perror("Child process"), 1);
	}
	close(pipefd[1]);
	ft_read_from_pipe(*fd, fd_rem);
	close_all(pipefd[0], fd_rem);
	*fd = 0;
	// close(fd_rem);
	// *fd = pipefd[0];
	return (0);
}

/*
	for piping process use array of fd (int fd[2]).
	fd[0] always for READING and f[1] for WRITING.
	fork() returns 0 only for child process, -1 for error
*/

int	pipex(int *fd, t_cmd *cmd, char **envp)
{
	int		pipefd[2];
	pid_t	id1;

	if (pipe(pipefd) == -1)
		return (perror("Pipe"), 1);
	id1 = fork();
	if (id1 < 0)
		return (perror("Fork"), 1);
	if (id1 == 0)
	{		
		close(pipefd[0]);
		if (dup2(*fd, STDIN_FILENO) < 0 || dup2(pipefd[1], STDOUT_FILENO) < 0)
			return (perror("Child process"), 1);
		execve(cmd->path, cmd->cmd, envp);
	}
	// ft_putstr_fd("in PIPEX parent proc\n", 2);
	close(pipefd[1]);
	*fd = pipefd[0];
	return (0);
}

static int	wait_all(int start, int end)
{
	int	status;

	while (start < end)
	{
		waitpid(-1, &status, 0);
		if (WIFEXITED(status) == 1)
		{
			if (WEXITSTATUS(status) == 1)
				return (1);
		}
		start++;
	}
	return (0);
}

static int	ft_read_from_pipe(int fd_in, int fd_out)
{
	char	*buf[BUFFER_SIZE];
	int		readed;

	if (dup2(fd_in, STDIN_FILENO) < 0 || dup2(fd_out, STDOUT_FILENO) < 0)
		return (perror("Read from pipe"), 1);
	readed = 1;
	while (readed != 0)
	{
		readed = read(STDIN_FILENO, buf, BUFFER_SIZE);
		if (readed == -1)
			return (perror("Reading"), 1);
		if (readed == 0)
			break ;
		else
		{
			if (write(STDOUT_FILENO, &buf, readed) <= 0)
				return (perror("Writing"), -1);
		}
	}
	return (0);
}
