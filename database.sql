-- Hospital Management System Database
-- Run this file to create and populate the database
-- Command: sqlite3 hospital.db < database.sql

CREATE TABLE IF NOT EXISTS patients (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL,
    age         INTEGER NOT NULL,
    gender      TEXT NOT NULL,
    disease     TEXT NOT NULL,
    doctor      TEXT NOT NULL,
    ward        TEXT NOT NULL,
    admitted    TEXT NOT NULL,
    status      TEXT DEFAULT 'Admitted'
);

CREATE TABLE IF NOT EXISTS doctors (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL,
    specialty   TEXT NOT NULL,
    phone       TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS appointments (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_id  INTEGER,
    doctor_id   INTEGER,
    date        TEXT NOT NULL,
    time        TEXT NOT NULL,
    reason      TEXT,
    FOREIGN KEY (patient_id) REFERENCES patients(id),
    FOREIGN KEY (doctor_id)  REFERENCES doctors(id)
);

-- Sample doctors
INSERT INTO doctors (name, specialty, phone) VALUES
    ('Dr. Sharma',   'General Medicine', '9876543210'),
    ('Dr. Mehta',    'Endocrinology',    '9876543211'),
    ('Dr. Reddy',    'Cardiology',       '9876543212'),
    ('Dr. Kapoor',   'Neurology',        '9876543213'),
    ('Dr. Patel',    'Orthopedics',      '9876543214');

-- Sample patients
INSERT INTO patients (name, age, gender, disease, doctor, ward, admitted, status) VALUES
    ('Rahul Sharma',  24, 'Male',   'Fever',        'Dr. Sharma', 'General',  '2026-04-20', 'Admitted'),
    ('Priya Verma',   30, 'Female', 'Diabetes',     'Dr. Mehta',  'Special',  '2026-04-18', 'Admitted'),
    ('Amit Kumar',    45, 'Male',   'Hypertension', 'Dr. Reddy',  'ICU',      '2026-04-15', 'Critical'),
    ('Sneha Rao',     28, 'Female', 'Migraine',     'Dr. Kapoor', 'General',  '2026-04-22', 'Admitted'),
    ('Vijay Singh',   55, 'Male',   'Fracture',     'Dr. Patel',  'Ortho',    '2026-04-21', 'Admitted'),
    ('Meena Joshi',   40, 'Female', 'Typhoid',      'Dr. Sharma', 'General',  '2026-04-23', 'Recovered');
