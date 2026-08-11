-- 学生管理系统建库脚本
CREATE DATABASE IF NOT EXISTS student_manage DEFAULT CHARACTER SET utf8mb4;
USE student_manage;

CREATE TABLE IF NOT EXISTS users (
  id INT AUTO_INCREMENT PRIMARY KEY,
  username VARCHAR(50) NOT NULL UNIQUE,
  password_hash CHAR(64) NOT NULL,
  salt CHAR(32) NOT NULL,
  role VARCHAR(20) NOT NULL DEFAULT 'admin',
  created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS teachers (
  id INT AUTO_INCREMENT PRIMARY KEY,
  teacher_no VARCHAR(20) NOT NULL UNIQUE,
  name VARCHAR(50) NOT NULL,
  gender CHAR(1) NOT NULL DEFAULT '男',
  title VARCHAR(20) NOT NULL DEFAULT '',
  phone VARCHAR(20) NOT NULL DEFAULT '',
  email VARCHAR(100) NOT NULL DEFAULT ''
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS classes (
  id INT AUTO_INCREMENT PRIMARY KEY,
  class_name VARCHAR(50) NOT NULL,
  grade INT NOT NULL,
  major VARCHAR(50) NOT NULL DEFAULT '',
  head_teacher_id INT,
  CONSTRAINT fk_classes_teacher FOREIGN KEY (head_teacher_id)
    REFERENCES teachers(id) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS students (
  id INT AUTO_INCREMENT PRIMARY KEY,
  student_no VARCHAR(20) NOT NULL UNIQUE,
  name VARCHAR(50) NOT NULL,
  gender CHAR(1) NOT NULL DEFAULT '男',
  birth_date DATE,
  phone VARCHAR(20) NOT NULL DEFAULT '',
  email VARCHAR(100) NOT NULL DEFAULT '',
  class_id INT,
  enroll_year INT,
  status VARCHAR(10) NOT NULL DEFAULT '在读',
  CONSTRAINT fk_students_class FOREIGN KEY (class_id)
    REFERENCES classes(id) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS courses (
  id INT AUTO_INCREMENT PRIMARY KEY,
  course_no VARCHAR(20) NOT NULL UNIQUE,
  course_name VARCHAR(50) NOT NULL,
  credit DECIMAL(3,1) NOT NULL DEFAULT 0,
  teacher_id INT,
  CONSTRAINT fk_courses_teacher FOREIGN KEY (teacher_id)
    REFERENCES teachers(id) ON DELETE RESTRICT
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

CREATE TABLE IF NOT EXISTS scores (
  id INT AUTO_INCREMENT PRIMARY KEY,
  student_id INT NOT NULL,
  course_id INT NOT NULL,
  semester VARCHAR(20) NOT NULL,
  score DECIMAL(5,2) NOT NULL,
  CONSTRAINT fk_scores_student FOREIGN KEY (student_id)
    REFERENCES students(id) ON DELETE RESTRICT,
  CONSTRAINT fk_scores_course FOREIGN KEY (course_id)
    REFERENCES courses(id) ON DELETE RESTRICT,
  UNIQUE KEY uk_scores (student_id, course_id, semester)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 默认管理员:admin / admin123(盐固定为 32 字符 hex,程序端 hashPassword 拼接方式必须一致)
INSERT IGNORE INTO users (username, password_hash, salt, role)
VALUES ('admin', SHA2(CONCAT('admin123', 'a1b2c3d4e5f6a7b8a1b2c3d4e5f6a7b8'), 256),
        'a1b2c3d4e5f6a7b8a1b2c3d4e5f6a7b8', 'admin');
